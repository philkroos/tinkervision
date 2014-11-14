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
    camera_user_count_.clear();
}

bool tfv::CameraControl::is_available(TFV_Id camera_id) {

    auto result = acquire(camera_id);
    safe_release(camera_id);

    return result;
}

bool tfv::CameraControl::acquire(TFV_Id camera_id) {

    auto result = false;
    auto camera = camera_map_.find(camera_id);

    // if new cam_id, add cam_id to map but don't open it yet
    if (camera == camera_map_.end()) {
        // open new; currently Opencv color-Usb-cams hardcoded
        TFV_Byte constexpr channels = 3;
        camera_map_[camera_id] = new CameraUsbOpenCv(camera_id, channels);
        camera = camera_map_.find(camera_id);
    }

    // only try to open cam if more users allowed for this id
    auto camera_user_count = camera_user_count_.find(camera_id);
    if (camera_user_count != camera_user_count_.end()) {

        // Number of configurations per cam is limited
        if (camera_user_count->second < max_users_per_cam_) {
            if (camera->second->is_open() or camera->second->open()) {
                camera_user_count->second++;
                result = true;
            }
        }
    } else if (camera->second->is_open() or camera->second->open()) {
        camera_user_count_[camera_id] = 1;
        result = true;
    }

    // if cam cam_id is not used by now, remove it from the map
    if (camera_user_count_.find(camera_id) == camera_user_count_.end()) {
        if (camera_map_[camera_id]) {
            camera_map_[camera_id]->stop();
            delete camera_map_[camera_id];
        }
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
    camera_user_count_.erase(camera_id);
}

// close
TFV_Byte tfv::CameraControl::safe_release(TFV_Id camera_id) {
    TFV_Byte users = 0;

    if (camera_user_count_.find(camera_id) != camera_user_count_.end()) {
        camera_user_count_[camera_id] =
            std::max(0, camera_user_count_[camera_id] - 1);
        users = camera_user_count_[camera_id];
    }

    auto camera = camera_map_.find(camera_id);

    // No more users? Free the cam!
    if (camera != camera_map_.end() and not users) {
        if (camera->second) {
            camera->second->stop();
            delete camera->second;
            camera_map_.erase(camera_id);
        }
    }
    return users;
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
