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

tfv::Camera::Camera(TFV_Id camera_id) : camera_id_(camera_id) {}

tfv::Camera::Camera(TFV_Id camera_id, uint_fast16_t requested_width,
                    uint_fast16_t requested_height)
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

    auto result = retrieve_frame(image);

    if (result) {

        // Todo? Let this be set by the actual camera module
        image.timestamp =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch())
                .count();

        image.format = image_format();
    }

    return result;
}

bool tfv::Camera::get_properties(uint_fast16_t& width, uint_fast16_t& height,
                                 size_t& framebytesize) {
    if (is_open()) {
        retrieve_properties(width, height, framebytesize);
        return true;
    }
    return false;
}

void tfv::Camera::stop(void) {
    active_ = false;
    close();
}
