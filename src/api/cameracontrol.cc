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

tfv::CameraControl::~CameraControl(void) { release_all(); }

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

bool tfv::CameraControl::preselect_framesize(uint16_t framewidth,
                                             uint16_t frameheight) {
    if (not usercount_) {
        requested_width_ = framewidth;
        requested_height_ = frameheight;
    }
    return not usercount_;
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

        {
            std::lock_guard<std::mutex> cam_mutex(camera_mutex_);
            open = _open_device();
        }
    }

    if (open) {
        usercount_++;
        Log("CAMERACONTROL::acquire", usercount_, " users.");
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

    usercount_ = std::max(usercount_ - 1, 0);

    if (not usercount_ and camera_) {
        Log("CAMERACONTROL", "Closing the device");
        std::lock_guard<std::mutex> camera_lock(camera_mutex_);
        _close_device();
    }
}

void tfv::CameraControl::stop_camera(void) {
    std::lock_guard<std::mutex> camera_lock(camera_mutex_);
    _close_device();
}

void tfv::CameraControl::release_all(void) {
    Log("CAMERACONTROL::release_all", "Closing with users: ", usercount_);
    while (usercount_) {
        release();
    }

    // in case usercount_ was already 0:
    _close_device();
}

bool tfv::CameraControl::get_properties(uint16_t& height, uint16_t& width,
                                        size_t& frame_bytesize) {
    auto result = false;

    if (is_open()) {
        result = camera_->get_properties(height, width, frame_bytesize);
    }

    return result;
}

bool tfv::CameraControl::get_resolution(uint16_t& width, uint16_t& height) {
    size_t bytesize;  // ignored
    return get_properties(width, height, bytesize);
}

tfv::Converter* tfv::CameraControl::get_converter(tfv::ColorSpace from,
                                                  tfv::ColorSpace to) {

    auto it = std::find_if(provided_formats_.begin(), provided_formats_.end(),
                           [&](Converter const& converter) {

        return (converter.source_format() == from) and
               (converter.target_format() == to);
    });

    if (it == provided_formats_.end()) {
        provided_formats_.emplace_back(from, to);
        it = --provided_formats_.end();
    }

    return &(*it);
}

void tfv::CameraControl::get_frame(tfv::Image& image, tfv::ColorSpace format) {

    // If the requested format is the same as provided by the camera,
    // image_.
    if (format == image_.format) {
        image = image_;
        return;
    }

    // Else, check if a converter for the requested format already is
    // instantiated. If not, insert a new one. Else, check if it contains a
    // valid result with the same timestamp as image_. If, it has been run
    // already, just return the result. Else, run the converter.

    auto converter = get_converter(image_.format, format);
    if (converter) {
        image = converter->result();

        if (image.format == tfv::ColorSpace::INVALID or
            image.timestamp != image_.timestamp) {

            // conversion
            image = (*converter)(image_);
        }
    }
}

void tfv::CameraControl::regenerate_image_from(Image const& image) {
    if (image.format != image_.format) {
        auto converter = get_converter(image.format, image_.format);
        if (not converter) {
            LogError("CAMERACONTROL", "Can't regenerate formats from ",
                     image.format, " (baseformat: ", image_.format, ")");
            return;
        }
        (*converter)(image, image_);
    }

    for (auto& converter : provided_formats_) {
        if (converter.target_format() == image_.format) {
            continue;
        }
        converter(image);
    }
}

bool tfv::CameraControl::update_frame(void) {

    {
        std::lock_guard<std::mutex> cam_lock(camera_mutex_);
        if (stopped_) {
            if (not _open_device()) {
                assert(stopped_);
                return false;
            }
        }

        if (not camera_) {
            if (fallback.active) {
                Log("CAMERACONTROL", "Fallback image");
                image_ = fallback.image;
            } else {
                return false;
            }

        } else if (not camera_->get_frame(image_)) {
            return false;
        }
    }

    if (not fallback.active) {
        image_.timestamp = Clock::now();
    }

    if (image_.format == tfv::ColorSpace::INVALID) {
        LogWarning("CAMERACONTROL", "INVALID image format");
        return false;
    }

    return true;
}

bool tfv::CameraControl::_open_device(void) {
    static const auto MAX_DEVICE = 5;
    auto i = int(MAX_DEVICE);

    // selecting the highest available device
    for (; i >= 0; --i) {
        if (_device_exists(i)) {

            Log("CAMERACONTROL", "Opening camera device ", i);
            camera_ = new V4L2USBCamera(i, requested_width_, requested_height_);
            // camera_ = new OpenCvUSBCamera(i);

            fallback.active = not camera_->open();
            if (not fallback.active) {
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

void tfv::CameraControl::_close_device() {

    // Save the last image in case it is requested again.
    // This is a precaution to prevent possible race conditions
    // and to simplify interface usage if being accessed from several
    // threads
    // without making things overcomplicated.

    if (camera_) {

        if (fallback.image.data) {
            delete[] fallback.image.data;
        }

        Image temp;
        fallback.active = camera_->get_frame(temp);

        if (fallback.active) {  // image retrieved

            // copies the image properties
            fallback.image = temp;

            // create a deep copy of the data
            fallback.image.data = new TFV_ImageData[fallback.image.bytesize];
            std::copy_n(temp.data, fallback.image.bytesize,
                        fallback.image.data);
        }
        camera_->stop();

        delete camera_;
        camera_ = nullptr;
    }

    stopped_ = true;
}
