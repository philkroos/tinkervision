/// \file v4l2_camera.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of a camera using the V4L2-interface.
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

#ifndef V4L2_CAMERA_H
#define V4L2_CAMERA_H

#ifndef WITH_OPENCV_CAM

#include <array>

#include <fcntl.h>
#include <cerrno>
#include <cstring>       // std::strerror
#include <system_error>  // std::errc

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>

#include <linux/videodev2.h>
#include <libv4l2.h>

// baseclass
#include "camera.hh"
#include "logger.hh"

// aliases for v4l2 types and functions plus definition of related types
namespace v4l2 {

// v4l2 types
using Requestbuffers = v4l2_requestbuffers;
using Buffer = v4l2_buffer;
using BufferType = v4l2_buf_type;
using BufferMemory = v4l2_memory;
using Format = v4l2_format;
using PixelFormat = v4l2_pix_format;
using FrameIntervalEnum = v4l2_frmivalenum;
using StreamParameter = v4l2_streamparm;
using Timeout = struct timeval;

// additional types
using Frame = struct Frame {
    void* start = nullptr;
    size_t length = 0;
    bool mapped = false;  // preventing unmap of unmapped
};

// constants needed in the V4L2USBCamera declaration
static v4l2_buf_type BUFFER_TYPE_VIDEO_CAPTURE = V4L2_BUF_TYPE_VIDEO_CAPTURE;
static auto BUFFER_MEMORY_MMAP = V4L2_MEMORY_MMAP;
// static auto YV12 = V4L2_PIX_FMT_YVU420;  // planar. Encoder-Accepted.
static auto YUYV = V4L2_PIX_FMT_YUYV;  // 422, packed

struct Request {
    long unsigned const value;
    char const* name;
};

//// Functor wrapping ioctl requests.  If evaluation yields false, the
/// occured error can be retrieved from result until the next
/// evaluation. It's the system errno, see std::errc and std::strerror)
class IOControl {
    inline bool _error_during_ioctl(void) const { return result == -1; }
    inline bool _interrupted(void) const { return errno == EINTR; }
    inline bool _unavailable(void) const { return errno == EAGAIN; }

public:
    bool operator()(int device_handle, Request const& request, void* arg) {

        do {
            result = v4l2_ioctl(device_handle, request.value, arg);
        } while (_error_during_ioctl() and (_interrupted() or _unavailable()));

        if (_error_during_ioctl()) {

            // Store a meaningfull error number for the caller
            result = errno;
        } else {
            result = 0;
        }

        return result == 0;
    }

    int result = 0;
};
}

namespace tv {

using frame_resolution = struct frame_resolution {
    const char* format;
    size_t width;
    size_t height;
};

using ColorSpaceMapping = struct ColorSpaceMapping {
    const unsigned v4l2_id;
    const tv::ColorSpace tv_id;
};

class V4L2USBCamera : public Camera {
public:
    explicit V4L2USBCamera(uint8_t camera_id);
    ~V4L2USBCamera(void) override final;

    bool open_device(void) override final;
    bool open_device(uint16_t, uint16_t) override final;

    bool is_open(void) const override final;
    ColorSpace image_format(void) const override final {
        return supported_codings_[coding_].tv_id;
    }

    bool select_best_available_settings(void);

protected:
    bool retrieve_frame(tv::ImageData** data) override final;
    void retrieve_properties(uint16_t& width, uint16_t& height,
                             size_t& frame_bytesize) override final;
    void close(void) override final;

private:
#ifdef DEBUG
    char const* v4l2_log = "/tmp/tv_v4l2.log";
#else
    char const* v4l2_log = "/dev/null";
#endif

    std::array<ColorSpaceMapping, 1> supported_codings_ = {{
        {v4l2::YUYV, ColorSpace::YUYV},
        // Got no hw supporting this to test it yet
        //{v4l2::YV12, ColorSpace::YV12},
    }};

    std::array<frame_resolution, 5> supported_resolutions_ = {{
        {"WUXGA", 1920, 1200},   // 16:10
        {"FullHD", 1920, 1080},  // 16:9, 1080p
        {"HDTV", 1280, 720},     // 16:9, 720p
        {"LD", 640, 480},        // 4:3
        {"VHS", 320, 240},       // 4:3
    }};

    int device_ = 0;  ///< camera device handle

    static const int seconds = 5;
    static const int microseconds = 0;
    v4l2::Timeout device_wait_timeout_{
        seconds,
        microseconds};  ///< Timeout during request (select) of video device

    static const int request_buffer_count_ =
        4;  ///< Frame buffers provided to v4l

    v4l2::Requestbuffers request_buffers_;

    v4l2::BufferType buffer_type_ =
        v4l2::BUFFER_TYPE_VIDEO_CAPTURE;  ///< recording a stream of images

    v4l2::BufferMemory buffer_memory_ =
        v4l2::BUFFER_MEMORY_MMAP;  ///< exchanging data with the driver via
    /// memory-mapped buffers

    v4l2::Buffer buffer_;  ///< 'Working' buffer; pointing to 'real' buffer
    v4l2::Frame* frames_ = nullptr;  ///< memory-mapped actual buffer

    size_t frame_width_ = 0;     ///< resolution width
    size_t frame_height_ = 0;    ///< resolution height
    size_t frame_bytesize_ = 0;  ///< size of data retrieved per frame

    int coding_ = -1;       ///< index into supported_codings_
    int resolution_ = -1;   ///< index into supported_resolutions_
    double framerate_ = 0;  ///< not settable currently
    bool running_ = false;  ///< True if capturing frames

    // helper
    bool _start_capturing(void);
    int _capture_frame_byte_size(void);
    bool _init_request_buffers(void);
    inline void _init_info_buffer(int index);
    bool _set_format_and_resolution(v4l2::Format& format, size_t format_index,
                                    size_t resolution_index);
    bool _select_requested_settings(uint16_t width, uint16_t height);
    bool _set_best_format_and_resolution(v4l2::Format& format);
    bool _set_highest_framerate(v4l2::PixelFormat& px_format);
    void _retrieve_properties(void);

    v4l2::IOControl io_control_;  ///< system ioctl abstraction

    // io_control_ wrapper
    bool io_operation(int const& device_handle, v4l2::Request const& request,
                      void* arg, int ignorable) {

        auto result = io_control_(device_handle, request, arg);
        if (not result) {
            LogError("V4L2_CAM", strerror(io_control_.result), " (",
                     io_control_.result, ") ", " for request ", request.name,
                     (ignorable == io_control_.result ? ": ignorable"
                                                      : ": not ignorable"));
        }
        return result;
    }

    bool io_operation(int const& device_handle, v4l2::Request const& request,
                      void* arg) {

        return io_operation(device_handle, request, arg, 0);
    }
};
}
#endif

#endif
