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

#include "camera.hh"

tfv::Camera::Camera(TFV_Id camera_id) : camera_id_(camera_id) {}

bool tfv::Camera::get_frame(tfv::Image& image) {
    if (not is_open()) {
        stop();
        return false;
    }

    return retrieve_frame(image);
}

bool tfv::Camera::get_properties(size_t& height, size_t& width,
                                 size_t& framebytesize) {
    if (is_open()) {
        retrieve_properties(height, width, framebytesize);
        return true;
    }
    return false;
}

void tfv::Camera::stop(void) {
    active_ = false;
    close();
}
