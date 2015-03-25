/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014 philipp.kroos@fh-bielefeld.de

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

#include "cameracontrol.hh"

#include <thread>
#include <chrono>

tfv::CameraControl::~CameraControl(void) { release(); }

bool tfv::CameraControl::is_available(void) {

    auto result = false;
    if (camera_ and (camera_->is_open() or camera_->open())) {

        result = true;  // already open

    } else {
        std::lock_guard<std::mutex> cam_mutex(camera_mutex_);
        if (_open_device()) {

            _close_device();
            result = true;  // opening possible
        }
    }
    return result;
}

bool tfv::CameraControl::acquire(size_t user) {
    auto result = false;

    if (user > 0) {
        result = acquire();

        if (user > 1) {  // acquire adds one user
            add_user(user - 1);
        }
    }
    return result;
}

bool tfv::CameraControl::acquire(void) {

    auto open = is_open();

    if (not open) {

        if (camera_) {
            release();
        }

        if (not stopped_) {
            std::lock_guard<std::mutex> cam_mutex(camera_mutex_);
            open = _open_device();
        }
    }

    if (open) {
        usercount_++;
    }

    return open;
}

bool tfv::CameraControl::is_open(void) {

    if (not camera_ or not camera_->is_open()) {
        if (camera_) {
            release();
        }
    }

    return camera_ != nullptr;
}

void tfv::CameraControl::release(void) {

    std::cout << "Release called" << std::endl;
    usercount_ = std::max(usercount_ - 1, 0);

    if (not usercount_ and camera_) {
        std::lock_guard<std::mutex> camera_lock(camera_mutex_);
        _close_device();
    }
}

void tfv::CameraControl::stop_camera(void) {
    std::lock_guard<std::mutex> camera_lock(camera_mutex_);
    stopped_ = true;
    _close_device();
}

bool tfv::CameraControl::get_properties(size_t& height, size_t& width,
                                        size_t& frame_bytesize) {
    auto result = false;

    if (is_open()) {
        result = camera_->get_properties(height, width, frame_bytesize);
    }

    return result;
}

bool tfv::CameraControl::get_frame(tfv::Image& image) {

    auto result = not stopped_;

    {
        std::lock_guard<std::mutex> cam_lock(camera_mutex_);

        if (stopped_) {
            stopped_ = false;
            result = _open_device();
        }

        if (result) {
            if (not camera_) {
                // std::cout << "Setting to fallback image" << std::endl;
                image = fallback.image;
                result = true;
            } else {
                result = camera_->get_frame(image);

                if (not result) {
                    // give it a little time and try again
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    result = camera_->get_frame(image);
                }
            }
        }
    }

    return result;
}

bool tfv::CameraControl::_open_device() {
    static const auto MAX_DEVICE = 5;
    auto i = 0;

    for (; i < MAX_DEVICE; ++i) {
        if (_device_exists(i)) {

            // camera_ = new V4L2USBCamera(i);
            camera_ = new OpenCvUSBCamera(i);

            fallback.active = not camera_->open();
            if (not fallback.active) {
                break;
            } else {
                delete camera_;
                camera_ = nullptr;
                continue;
            }
        }
    }
    return i < MAX_DEVICE;
}

void tfv::CameraControl::_close_device() {

    // Save the last image in case it is requested again.
    // This is a precaution to prevent possible race conditions
    // and simplify interface usage if being accessed from several threads
    // without making things overcomplicated here.
    if (camera_) {
        fallback.active = camera_->get_frame(fallback.image);
        camera_->stop();

        delete camera_;
        camera_ = nullptr;
    }
}
