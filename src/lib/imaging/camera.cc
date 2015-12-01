/// \file camera.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Implementation of Camera.
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

#include "camera.hh"

#include <chrono>

#include "logger.hh"

tv::Camera::Camera(uint8_t camera_id) : camera_id_(camera_id) {}

bool tv::Camera::get_frame(tv::Image& image) {
    if (not is_open()) {
        stop();
        return false;
    }

    if (not retrieve_frame(&image_.data)) {
        return false;
    }

    image = image_;
    return true;
}

bool tv::Camera::get_properties(uint16_t& width, uint16_t& height,
                                size_t& framebytesize) {
    if (is_open()) {
        retrieve_properties(width, height, framebytesize);
        return true;
    }
    return false;
}

bool tv::Camera::open(void) { return open(0, 0); }

bool tv::Camera::open(uint16_t width, uint16_t height) {
    auto success = (width ? open_device(width, height) : open_device());

    if (success) {
        active_ = true;
        auto& header = image_.header;
        retrieve_properties(header.width, header.height, header.bytesize);
        header.format = image_format();
        Log("CAMERA", "Opened camera ", camera_id_, ": ", header);
    }
    return success;
}

void tv::Camera::stop(void) {
    active_ = false;
    close();
    Log("CAMERA", "Closed camera ", camera_id_);
}
