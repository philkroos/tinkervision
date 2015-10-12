/*
 Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
 Copyright (C) 2014-2015 philipp.kroos@fh-bielefeld.de

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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

tv::CameraControl::~CameraControl(void) { release_all(); }

tv::CameraControl::CameraControl(void) {
    fallback_.allocate(640, 480, 640 * 480 * 3, ColorSpace::BGR888, false);
    std::fill_n(fallback_().data, fallback_().header.bytesize, 255);
}

bool tv::CameraControl::is_available(void) {

    if (camera_ and camera_->is_open()) {

        return true;
    }

    return _test_device();
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

    if (not _init()) {
        requested_width_ = old_width;
        requested_height_ = old_height;
        return false;
    }

    stop_camera();

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
        _close_device();
    }

    return open;
}

bool tv::CameraControl::is_open(void) {

    if (not camera_ or not camera_->is_open()) {
        if (camera_) {
            release();
        }
    }

    return camera_ != nullptr;
}

void tv::CameraControl::release(void) {

    usercount_ = std::max(usercount_ - 1, 0);

    if (not usercount_ and camera_) {
        Log("CAMERACONTROL", "Closing the device");
        std::lock_guard<std::mutex> camera_lock(camera_mutex_);
        _close_device();
    }
}

void tv::CameraControl::stop_camera(void) {
    std::lock_guard<std::mutex> camera_lock(camera_mutex_);
    _close_device();
}

void tv::CameraControl::release_all(void) {
    Log("CAMERACONTROL::release_all", "Closing with users: ", usercount_);
    while (usercount_) {
        release();
    }

    // in case usercount_ was already 0:
    _close_device();
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

    if (not _update_from_camera()) {
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

    if (not _open_device()) {
        return false;
    }

    _close_device();
    return true;
}

bool tv::CameraControl::_init(void) {
    std::lock_guard<std::mutex> cam_mutex(camera_mutex_);

    auto success = _open_device();
    if (success) {
        image_.allocate(camera_->frame_header(), false);
    }

    return success;
}

bool tv::CameraControl::_open_device(void) {
    static const auto MAX_DEVICE = 5;
    auto i = int(MAX_DEVICE);

    // selecting the highest available device
    for (; i >= 0; --i) {
        if (_device_exists(i)) {

#ifdef WITH_OPENCV_CAM
            Log("CAMERACONTROL", "Opening OpenCV camera device ", i);
            camera_ = new OpenCvUSBCamera(i);
#else
            Log("CAMERACONTROL", "Opening V4L2 camera device ", i);
            camera_ = new V4L2USBCamera(i);
#endif

            if (camera_->open(requested_width_, requested_height_)) {
                stopped_ = false;
                break;
            } else {
                delete camera_;
                camera_ = nullptr;
                continue;
            }
        }
    }
    return i >= 0;
}

void tv::CameraControl::_close_device() {

    if (camera_) {

        camera_->stop();

        /// \todo It is not necessary to always delete the cam.
        delete camera_;
        camera_ = nullptr;
    }

    stopped_ = true;
}
