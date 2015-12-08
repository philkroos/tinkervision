/// \file v4l2_camera.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Definition of a camera using the V4L2-interface.
///
/// This file is part of Tinkervision - Vision Library for Tinkerforge Redbrick
/// \sa https://github.com/Tinkerforge/red-brick
///
/// \copyright
///
/// This program is free software; you can redistribute it and/or
/// modify it under the terms of the GNU General Public License
/// as published by the Free Software Foundation; either version 2
/// of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
/// USA.

#ifndef WITH_OPENCV_CAM

#include <cstring>  // memset

// cam access
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <cstdio>

#include "v4l2_camera.hh"

namespace v4l2 {

static auto TIME_PER_FRAME = V4L2_CAP_TIMEPERFRAME;
static auto DISCRETE_INTERVAL = V4L2_FRMIVAL_TYPE_DISCRETE;
static auto PROGRESSIVE = V4L2_FIELD_NONE;

// errors
static auto INVALID_VALUE = EINVAL;

static Request get_parameter = {VIDIOC_G_PARM, "'get parameter'"};
static Request set_parameter = {VIDIOC_S_PARM, "'set parameter'"};
static Request get_format = {VIDIOC_G_FMT, "'get format'"};
static Request set_format = {VIDIOC_S_FMT, "'set format'"};
static Request enumerate_frameintervals = {VIDIOC_ENUM_FRAMEINTERVALS,
                                           "'enumerate frameintervals'"};
static Request request_buffers = {VIDIOC_REQBUFS, "'request buffers'"};
static Request deque_buffers = {VIDIOC_DQBUF, "'deque buffers'"};
static Request queue_buffers = {VIDIOC_QBUF, "'queue buffers'"};
static Request stream_on = {VIDIOC_STREAMON, "'stream on'"};
static Request stream_off = {VIDIOC_STREAMOFF, "'stream off'"};
static Request query_buffers = {VIDIOC_QUERYBUF, "'query buffers'"};

// functions
static auto mmap = v4l2_mmap;
static auto munmap = v4l2_munmap;
static auto open = v4l2_open;
static auto close = v4l2_close;
}

tv::V4L2USBCamera::V4L2USBCamera(uint8_t camera_id) : Camera(camera_id) {
    // zero-initialize buffers for the frames to be grabbed
    frames_ = new v4l2::Frame[request_buffer_count_ * sizeof(v4l2::Frame)]();
    v4l2_log_file = fopen(v4l2_log, "a");
    if (v4l2_log_file) {
        Log("V4L2", "Opened logfile ", v4l2_log);
    } else {
        Log("V4L2", "Failed to open logfile ", v4l2_log, ": ", errno);
    }
}

tv::V4L2USBCamera::~V4L2USBCamera(void) {

    close();

    // un-mmap buffers
    if (frames_) {
        for (size_t i = 0; i < request_buffer_count_; ++i) {
            v4l2::munmap(frames_[i].start, frames_[i].length);
            frames_[i].mapped = false;
        }
    }

    if (v4l2_log_file) {
        fclose(v4l2_log_file);
    }
    delete[] frames_;
}

bool tv::V4L2USBCamera::is_open(void) const { return device_ != 0; }

bool tv::V4L2USBCamera::open_device(void) {
    return is_open() or open_device(0, 0);
}

bool tv::V4L2USBCamera::open_device(uint16_t width, uint16_t height) {

    if (is_open()) {
        return false;
    }

    const char* device =
        (std::string("/dev/video") + std::to_string(camera_id_)).c_str();

    device_ = v4l2::open(device, O_RDWR, 0);
    Log("V4L2", "Open ", std::string(device), ": ", device_);

    if (device_ == -1) {
        LogError("V4L2", "Open failed: ", strerror(errno));
        device_ = 0;
    }

    if (device_) {
        auto open = false;
        if (width != 0) {
            open = _select_requested_settings(width, height) and
                   _start_capturing();

        } else {
            open = select_best_available_settings() and _start_capturing();
        }
        if (not open) {
            close();
        }
    }

    return is_open();
}

void tv::V4L2USBCamera::retrieve_properties(uint16_t& framewidth,
                                            uint16_t& frameheight,
                                            size_t& frame_bytesize) {

    if (not frame_width_ and is_open()) {
        _retrieve_properties();
    }

    framewidth = frame_width_;
    frameheight = frame_height_;
    frame_bytesize = frame_bytesize_;
}

void tv::V4L2USBCamera::_retrieve_properties(void) {

    if (not frame_width_ and is_open()) {
        v4l2::Format format;

        format.type = buffer_type_;

        auto result = io_operation(device_, v4l2::get_format, &format);
        if (result) {
            frame_width_ = format.fmt.pix.width;
            frame_height_ = format.fmt.pix.height;
            frame_bytesize_ =
                format.fmt.pix.bytesperline * format.fmt.pix.height;
        }
    }
}

bool tv::V4L2USBCamera::_select_requested_settings(uint16_t width,
                                                   uint16_t height) {
    auto ok = false;

    if (is_open()) {  // Todo: errornumber?

        v4l2::Format format;
        format.type = buffer_type_;

        for (size_t i = 0; i < supported_resolutions_.size(); ++i) {
            if ((supported_resolutions_[i].width == width) and
                (supported_resolutions_[i].height == height)) {

                ok = _set_format_and_resolution(
                    format, 0, i);  // only one format currently
            }
        }
        if (ok) {
            (void)_set_highest_framerate(format.fmt.pix);
        }
    }

    return ok;
}

bool tv::V4L2USBCamera::select_best_available_settings(void) {
    auto result = false;

    if (is_open()) {  // Todo: errornumber?

        v4l2::Format format;
        format.type = buffer_type_;

        // priority 1,2,3: pixelformat, resolution, fps
        if (not _set_best_format_and_resolution(format)) {

            // found no resolution at all
            coding_ = resolution_ = -1;
            close();

        } else {

            // Not possible to set the framerate? Ok.
            (void)_set_highest_framerate(format.fmt.pix);
            result = true;
        }
    }
    return result;
}

bool tv::V4L2USBCamera::_set_format_and_resolution(v4l2::Format& format,
                                                   size_t format_index,
                                                   size_t resolution_index) {
    auto& px_format = format.fmt.pix;
    px_format.width = supported_resolutions_[resolution_index].width;
    px_format.height = supported_resolutions_[resolution_index].height;
    px_format.pixelformat = supported_codings_[format_index].v4l2_id;
    px_format.field = v4l2::PROGRESSIVE;
    px_format.bytesperline = 0;  // lets the driver set it

    if (io_operation(device_, v4l2::set_format, &format)) {

        coding_ = format_index;
        resolution_ = resolution_index;
    }

    return (coding_ == static_cast<int>(format_index)) and
           (resolution_ == static_cast<int>(resolution_index));
}

bool tv::V4L2USBCamera::_set_best_format_and_resolution(v4l2::Format& format) {

    auto& px_format = format.fmt.pix;

    // Using the best available, top-down approach
    // priority 1: pixelformat
    // priority 2: resolution
    coding_ = -1;
    resolution_ = -1;
    for (size_t i = 0; i < supported_resolutions_.size(); i++) {
        for (size_t j = 0; j < supported_codings_.size(); j++) {
            px_format.width = supported_resolutions_[i].width;
            px_format.height = supported_resolutions_[i].height;
            px_format.pixelformat = supported_codings_[j].v4l2_id;
            px_format.field = v4l2::PROGRESSIVE;
            px_format.bytesperline = 0;  // lets the driver set it
            if (io_operation(device_, v4l2::set_format, &format)) {

                coding_ = j;
                break;
            }
        }
        if (coding_ != -1) {

            // driver can have set different format than requested
            if (px_format.width == supported_resolutions_[i].width and
                px_format.height == supported_resolutions_[i].height) {

                resolution_ = i;
                break;
            } else {
                coding_ = -1;
            }
        }
    }

    return resolution_ != -1 and coding_ != -1;
}

bool tv::V4L2USBCamera::_set_highest_framerate(v4l2::PixelFormat& px_format) {
    // Assumes already selected framesize

    // if not supported by device, just use current setting
    v4l2::FrameIntervalEnum frame_interval;

    frame_interval.pixel_format = px_format.pixelformat;
    frame_interval.width = px_format.width;
    frame_interval.height = px_format.height;

    auto result = false;

    int max_numerator = 0;
    int max_denominator = 1;  // prevent /0
    framerate_ = 0.0;

    // enumerate possibilities
    for (int i = 0;; i++) {

        frame_interval.index = i;

        // ignore error message for invalid value which will be returned
        // eventually
        result = io_operation(device_, v4l2::enumerate_frameintervals,
                              &frame_interval, v4l2::INVALID_VALUE);

        if (not result) {
            break;
        }

        if (frame_interval.type != v4l2::DISCRETE_INTERVAL) {
            // Only discrete framerates supported
            continue;
        }

        auto const& discrete = frame_interval.discrete;
        auto const fps = discrete.denominator / (double)discrete.numerator;

        if (fps > framerate_) {
            max_numerator = discrete.numerator;
            max_denominator = discrete.denominator;
            framerate_ = fps;
        }
    }

    v4l2::StreamParameter stream_parameter;
    stream_parameter.type = v4l2::BUFFER_TYPE_VIDEO_CAPTURE;

    if (io_operation(device_, v4l2::get_parameter, &stream_parameter) and
        (stream_parameter.parm.capture.capability & v4l2::TIME_PER_FRAME)) {

        stream_parameter.parm.capture.timeperframe.numerator = max_numerator;
        stream_parameter.parm.capture.timeperframe.denominator =
            max_denominator;

        // return result of trying to set the framerate
        result = io_operation(device_, v4l2::set_parameter, &stream_parameter);
        Log("V4L2", "Set framerate to ", framerate_);
    }

    else {  // return can't set the framerate explicitly

        Log("V4L2", "Can't set the framerate");
        result = false;
    }

    return result;
}

void tv::V4L2USBCamera::close(void) {
    if (is_open()) {
        if (device_) {
            (void)io_operation(device_, v4l2::stream_off, &buffer_type_);
            // ignore for now
        }

        v4l2::close(device_);
        device_ = 0;
    }

    running_ = false;
}

bool tv::V4L2USBCamera::retrieve_frame(tv::ImageData** data) {
    auto result = false;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(device_, &fds);

    // device ready?
    result =
        (select(device_ + 1, &fds, NULL, NULL, &device_wait_timeout_) != -1);

    if (result) {
        // Queue last read buffer
        if (running_) {  // start with a dequeuing
            result = io_operation(device_, v4l2::queue_buffers, &buffer_);
        } else {
            running_ = true;
        }
    }

    if (result) {
        _init_info_buffer(0);  // index does not matter here as being set next

        // retrieve frame
        result = io_operation(device_, v4l2::deque_buffers, &buffer_);
    }

    if (result) {
        *data = static_cast<uint8_t*>(frames_[buffer_.index].start);
    }

    return result;
}

bool tv::V4L2USBCamera::_start_capturing(void) {
    Log("V4L2", "StartCapturing");
    auto result = false;

    if (is_open() and _init_request_buffers()) {
        result = true;

        // Initialize (map) framebuffers
        for (size_t i = 0; i < request_buffers_.count; ++i) {

            auto& frame = frames_[i];
            _init_info_buffer(i);

            result = io_operation(device_, v4l2::query_buffers, &buffer_);
            if (not result) {
                break;
            }

            frame.length = buffer_.length;
            frame.start =
                v4l2::mmap(nullptr, buffer_.length, PROT_READ | PROT_WRITE,
                           MAP_SHARED, device_, buffer_.m.offset);
            frame.mapped = true;

            if (MAP_FAILED == frame.start) {
                // Error mmapping buffers
                // see in errno
                result = false;
                break;
            }
        }

        if (result) {
            // queue all buffers for data exchange with driver
            for (size_t i = 0; i < request_buffers_.count; ++i) {
                _init_info_buffer(i);

                result = io_operation(device_, v4l2::queue_buffers, &buffer_);
                if (not result) {
                    break;
                }
            }
        }

        // begin capturing
        if (result) {
            _init_info_buffer(0);
            result = io_operation(device_, v4l2::stream_on, &buffer_type_);
        }
    }

    return result;
}

bool tv::V4L2USBCamera::_init_request_buffers(void) {
    std::memset(&request_buffers_, 0, sizeof(request_buffers_));
    request_buffers_.count = request_buffer_count_;
    request_buffers_.type = buffer_type_;
    request_buffers_.memory = buffer_memory_;
    return io_operation(device_, v4l2::request_buffers, &request_buffers_);
}

inline void tv::V4L2USBCamera::_init_info_buffer(int index) {
    std::memset(&buffer_, 0, sizeof(v4l2::Buffer));
    buffer_.type = buffer_type_;
    buffer_.memory = buffer_memory_;
    buffer_.index = index;
}

int tv::V4L2USBCamera::_capture_frame_byte_size(void) {
    v4l2::Format format;

    format.type = buffer_type_;
    if (not io_operation(device_, v4l2::get_format, &format)) {
        return -1;
    }

    return format.fmt.pix.bytesperline * format.fmt.pix.height;
}

#endif
