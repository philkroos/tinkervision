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

tfv::CameraControl::~CameraControl(void) { release_all(); }

void tfv::CameraControl::release_all(void) {

    for (auto& camera : camera_map_) {
        if (camera.second) {

            camera.second->stop();
            delete camera.second;
        }
    }

    camera_map_.clear();
}

bool tfv::CameraControl::is_available(TFV_Id camera_id) {

    auto result = false;
    if (camera_map_.find(camera_id) != camera_map_.end()) {

        auto cam = camera_map_[camera_id];
        if (cam and (cam->is_open() or cam->open())) {

            result = true;  // already open
        }
    } else if (acquire(camera_id)) {

        release(camera_id);
        result = true;  // opening possible
    }
    return result;
}

bool tfv::CameraControl::acquire(TFV_Id camera_id) {

    auto result = false;
    auto camera = camera_map_.find(camera_id);

    if (camera == camera_map_.end()) {
        // open new; currently Opencv color-Usb-cams hardcoded

        if (device_exists(camera_id)) {
            camera_map_[camera_id] = new CameraUsbOpenCv(camera_id);
            camera = camera_map_.find(camera_id);
        }
    }

    if (camera != camera_map_.end() and camera->second and
        (camera->second->is_open() or camera->second->open())) {

        result = true;
    } else if (camera != camera_map_.end() and camera->second) {
        delete camera->second;
        camera_map_.erase(camera_id);
    }

    return result;
}

bool tfv::CameraControl::is_open(TFV_Id camera_id) {

    auto camera = camera_map_.find(camera_id);
    return camera != camera_map_.end() and camera->second->is_open();
}

void tfv::CameraControl::release(TFV_Id camera_id) {

    auto camera = camera_map_.find(camera_id);

    if (camera != camera_map_.end()) {
        if (camera->second) {

            camera->second->stop();
            delete camera->second;
        }
    }

    camera_map_.erase(camera_id);
}

bool tfv::CameraControl::get_properties(TFV_Id camera_id, int& height,
                                        int& width, int& channels) {
    auto camera = camera_map_.find(camera_id);
    if (camera == camera_map_.end()) {

        return false;
    }
    return camera->second->get_properties(height, width, channels);
}

bool tfv::CameraControl::get_frame(TFV_Id camera_id, TFV_ImageData* frame) {
    auto camera = camera_map_.find(camera_id);
    if (camera == camera_map_.end()) {

        return false;
    }
    return camera->second->get_frame(frame);
}
