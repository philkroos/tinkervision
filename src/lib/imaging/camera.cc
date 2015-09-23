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

#include "camera.hh"

#include <chrono>

#include "logger.hh"

tfv::Camera::Camera(TFV_Id camera_id) : camera_id_(camera_id) {}

tfv::Camera::Camera(TFV_Id camera_id, uint16_t requested_width,
                    uint16_t requested_height)
    : camera_id_{camera_id},
      requested_width_{requested_width},
      requested_height_{requested_height} {

    if (not requested_height) {
        requested_width_ = 0;
    }
}

bool tfv::Camera::get_frame(tfv::Image& image) {
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

bool tfv::Camera::get_properties(uint16_t& width, uint16_t& height,
                                 size_t& framebytesize) {
    if (is_open()) {
        retrieve_properties(width, height, framebytesize);
        return true;
    }
    return false;
}

bool tfv::Camera::open(void) {
    auto success = open_device();
    if (success) {
        active_ = true;
        auto& header = image_.header;
        retrieve_properties(header.width, header.height, header.bytesize);
        header.format = image_format();
        Log("CAMERA", "Opened camera ", header);
    }
    return success;
}

void tfv::Camera::stop(void) {
    active_ = false;
    close();
}
