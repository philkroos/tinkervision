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
#include <iostream>

#include "cameracontrol.hh"

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

bool tfv::CameraControl::preselect_framesize(uint_fast16_t framewidth,
                                             uint_fast16_t frameheight) {
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

        if (not stopped_) {
            std::lock_guard<std::mutex> cam_mutex(camera_mutex_);
            open = _open_device();
        }
    }

    if (open) {
        usercount_++;
        // std::cout << "Now " << usercount_ << " users." << std::endl;
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
    // std::cout << usercount_ << " users left" << std::endl;

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

void tfv::CameraControl::release_all(void) {
    while (usercount_) {
        release();
    }
}

bool tfv::CameraControl::get_properties(uint_fast16_t& height, uint_fast16_t& width,
                                        size_t& frame_bytesize) {
    auto result = false;

    if (is_open()) {
        result = camera_->get_properties(height, width, frame_bytesize);
    }

    return result;
}

bool tfv::CameraControl::get_resolution(uint_fast16_t& width, uint_fast16_t& height) {
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

void tfv::CameraControl::regenerate_formats_from(Image const& image) {
    // std::cout << "Request to regenerate for " << image.format << std::endl;
    if (image.format != image_.format) {
        auto converter = get_converter(image.format, image_.format);
        if (not converter) {
            std::cout << "Can't regenerate formats from " << image.format
                      << " (baseformat: " << image_.format << ")" << std::endl;
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

    auto result = not stopped_;

    {
        std::lock_guard<std::mutex> cam_lock(camera_mutex_);

        if (stopped_) {
            stopped_ = false;
            result = _open_device();
        }

        if (result) {
            if (not camera_) {
                if (fallback.active) {
                    image_ = fallback.image;
                }
                result = fallback.active;

            } else {
                result = camera_->get_frame(image_);

                if (not result) {
                    // give it a little time, then give it a second try
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    result = camera_->get_frame(image_);
                }
            }
        }
    }

    if (image_.format == tfv::ColorSpace::INVALID) {
        std::cout << "Warning: INVALID image format" << std::endl;
    }

    return result;
}

bool tfv::CameraControl::_open_device(void) {
    static const auto MAX_DEVICE = 5;
    auto i = 0;

    // selecting the first available device
    for (; i < MAX_DEVICE; ++i) {
        if (_device_exists(i)) {

            camera_ = new V4L2USBCamera(i, requested_width_, requested_height_);
            // camera_ = new OpenCvUSBCamera(i);

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
}
