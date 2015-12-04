/// \file cameracontrol.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Defines CameraControl.
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

#include <thread>
#include <chrono>
#include <fstream>

#include "cameracontrol.hh"
#include "logger.hh"
#ifdef WITH_OPENCV_CAM
#include "opencv_camera.hh"
#else
#include "v4l2_camera.hh"
#endif

#include "filesystem.hh"

tv::CameraControl::~CameraControl(void) { release_all(); }

tv::CameraControl::CameraControl(void) noexcept {
    try {
        if (fallback_.allocate(640, 480, 640 * 480 * 3, ColorSpace::BGR888,
                               false)) {
            std::fill_n(fallback_().data, fallback_().header.bytesize, 255);
        }
    } catch (...) {
        LogError("CAMERA_CONTROL", "Error allocating fallback image");
    }
}

bool tv::CameraControl::is_available(void) {

    return (camera_ and camera_->is_open()) or _test_device();
}

bool tv::CameraControl::is_available(uint8_t id) {

    if (camera_ and camera_->id() == id) {  // correct camera currently selected
        return camera_->is_open() or _test_device();
    }

    Camera* tmp{nullptr};
    auto result = _test_device(&tmp, id);
    if (tmp) {
        delete tmp;
    }

    Log("CAMERA_CONTROL", "Device ", id, " available: ", result);
    return result;
}

bool tv::CameraControl::prefer(uint8_t id) {
    /// Sets preferred_device_ to id, no matter if that device is available or
    /// not.  Does not switch the open device, this needs to be done manually.
    if (is_available(id)) {
        preferred_device_ = id;
        return true;
    }
    return false;
}

bool tv::CameraControl::switch_to_preferred(uint8_t id) {
    std::lock_guard<std::mutex> cam_mutex(camera_mutex_);

    auto open = is_open();
    if (prefer(id)) {

        /// If the currently opened device is the same, do nothing else. Else,
        /// stop_camera() and call init() again, effectively opening the
        /// preferred_device_ or, if impossible, any other.
        if (is_open() and camera_->id() != id) {
            stop_camera();
            _init();
        }
    }
    auto open_now = is_open();
    assert(open == open_now);  // never change overall camera status here

    if (open_now) {
        return camera_->id() == id;
    }

    return not open;  // never close open cam here
}

bool tv::CameraControl::preselect_framesize(uint16_t framewidth,
                                            uint16_t frameheight) {
    if (is_open()) {
        return false;
    }

    auto old_width = requested_width_;
    auto old_height = requested_height_;

    requested_width_ = framewidth;
    requested_height_ = frameheight;

    {
        std::lock_guard<std::mutex> cam_mutex(camera_mutex_);

        if (not _init()) {
            requested_width_ = old_width;
            requested_height_ = old_height;
            return false;
        } else {
            uint16_t width, height;
            size_t bytesize;
            camera_->get_properties(width, height, bytesize);

            stop_camera();

            if ((width != framewidth) or (height != frameheight)) {
                requested_width_ = old_width;
                requested_height_ = old_height;
                return false;
            }
        }
    }
    return true;
}

bool tv::CameraControl::acquire(size_t user) {
    auto result = false;

    if (user > 0) {
        result = acquire();

        if (user > 1) {  // acquire adds one user
            add_user(user - 1);
        }
    }
    return result;
}

bool tv::CameraControl::acquire(void) {
    std::lock_guard<std::mutex> cam_mutex(camera_mutex_);

    auto open = is_open();

    if (not open) {

        if (camera_) {
            release();
        }

        // on first open, get a frame to learn the header.
        open = _init();
    }

    if (open) {
        usercount_++;
        Log("CAMERACONTROL::acquire", usercount_, " users.");
    } else {
        _close_device(&camera_);
    }

    return open;
}

bool tv::CameraControl::is_open(void) const {

    if (not camera_ or not camera_->is_open()) {
        assert(not camera_);
        // 12-01-2015: To make this method const.
        // if (camera_) {
        //     release();
        // }
    }

    return camera_ != nullptr;
}

int16_t tv::CameraControl::current_device(void) const {
    return not is_open() ? -1 : static_cast<uint16_t>(camera_->id());
}

void tv::CameraControl::release(void) {

    usercount_ = std::max(usercount_ - 1, 0);

    if (not usercount_ and camera_) {
        Log("CAMERACONTROL", "Closing the device");
        std::lock_guard<std::mutex> camera_lock(camera_mutex_);
        _close_device(&camera_);
    }
}

void tv::CameraControl::stop_camera(void) {
    Log("CAMERA_CONTROL", "Stop");
    _close_device(&camera_);
}

void tv::CameraControl::release_all(void) {
    Log("CAMERACONTROL::release_all", "Closing with users: ", usercount_);
    while (usercount_) {
        release();
    }

    // in case usercount_ was already 0:
    _close_device(&camera_);
}

bool tv::CameraControl::get_properties(uint16_t& height, uint16_t& width,
                                       size_t& frame_bytesize) {
    return is_open() and camera_->get_properties(height, width, frame_bytesize);
}

bool tv::CameraControl::get_resolution(uint16_t& width, uint16_t& height) {
    size_t bytesize;  // ignored
    return get_properties(width, height, bytesize);
}

bool tv::CameraControl::update_frame(Image& image) {

    if (stopped_) {
        if (not _init()) {
            assert(stopped_);
            return false;
        }
    }

    // camera_ is open
    if (not _update_from_camera()) {
        if (not fallback_.image().data) {
            LogError("CAMERA_CONTROL", "No valid image");
            return false;
        }
        image_.set_from_image(fallback_.image());
    }

    if (image_().header.format == tv::ColorSpace::INVALID) {
        LogWarning("CAMERACONTROL", "INVALID image format");
        return false;
    }

    image = image_();
    return true;
}

bool tv::CameraControl::_update_from_camera(void) {
    {
        std::lock_guard<std::mutex> cam_lock(camera_mutex_);
        Image image;
        if (not camera_ or not camera_->get_frame(image)) {
            return false;
        }
        image_.copy_data(image.data, image.header.bytesize);
    }

    image_.image().header.timestamp = Clock::now();
    return true;
}

bool tv::CameraControl::_test_device(void) {
    std::lock_guard<std::mutex> cam_mutex(camera_mutex_);

    if (not _open_device(&camera_)) {
        return false;
    }

    _close_device(&camera_);
    return true;
}

bool tv::CameraControl::_test_device(Camera** cam, uint8_t device) {
    /// This assumes that device is not the currently used camera, if any.
    /// E.g., if camera_ is not nullptr and it is the same id,
    /// _test_device()
    /// must be used, which locks access to the device in use.

    assert(not*cam or (*cam != camera_));
    if (not _open_device(cam, device)) {
        return false;
    }

    _close_device(cam);
    return true;
}

bool tv::CameraControl::_init(void) {
    Log("CAMERA_CONTROL", "Init");

    /// Open the preferred_device_ if set, or any other.
    auto success = false;
    if (device_preferred()) {
        Log("CAMERA_CONTROL", "Init with selected id ", preferred_device_);
        success =
            _open_device(&camera_, preferred_device_) or _open_device(&camera_);
    } else {
        Log("CAMERA_CONTROL", "Init with any device");
        success = _open_device(&camera_);
    }
    if (success) {
        image_.allocate(camera_->frame_header(), false);
    }

    return success;
}

bool tv::CameraControl::_open_device(Camera** device) {
    static const auto MAX_DEVICE = 5;
    auto i = uint8_t(MAX_DEVICE);

    std::string device_name("/dev/video");
    // selecting the highest available device
    for (; i >= 0; --i) {
        device_name += std::to_string(i);
        if (is_cdevice(device_name)) {

#ifdef WITH_OPENCV_CAM
            Log("CAMERACONTROL", "Opening OpenCV camera device ", i);
            *device = new OpenCvUSBCamera(i);
#else
            Log("CAMERACONTROL", "Opening V4L2 camera device ", i);
            *device = new V4L2USBCamera(i);
#endif

            if ((*device)->open(requested_width_, requested_height_)) {
                stopped_ = false;
                return true;
            } else {
                (*device)->stop();
                delete *device;
                (*device) = nullptr;
            }
        }
        device_name.pop_back();
    }
    return false;
}

bool tv::CameraControl::_open_device(Camera** device, uint8_t id) {
    if (not is_cdevice("/dev/video" + std::to_string(id))) {
        return false;
    }

#ifdef WITH_OPENCV_CAM
    Log("CAMERACONTROL", "Opening OpenCV camera device ", id);
    *device = new OpenCvUSBCamera(id);
#else
    Log("CAMERACONTROL", "Opening V4L2 camera device ", id);
    *device = new V4L2USBCamera(id);
#endif
    if (not(*device)->open(requested_width_, requested_height_)) {
        delete *device;
        (*device) = nullptr;
        return false;
    }

    // device active
    stopped_ = false;
    return true;
}

void tv::CameraControl::_close_device(Camera** device) {

    auto stop = *device == camera_;
    if (*device) {

        (*device)->stop();
        delete *device;
        (*device) = nullptr;
    }
    /// If device equals camera_, the publicly available device will be
    /// stopped.
    if (stop) {
        stopped_ = true;
    }
}
