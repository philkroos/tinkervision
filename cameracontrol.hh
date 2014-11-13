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

#ifndef CAMERACONTROL_H
#define CAMERACONTROL_H

#include <opencv2/opencv.hpp>

#include "camera.hh"

namespace tfv {

class CameraControl {
public:
    explicit CameraControl(TFV_Id max_users_per_cam)
        : max_users_per_cam_{max_users_per_cam} {}

    ~CameraControl(void);

    CameraControl(CameraControl const&) = delete;
    CameraControl& operator=(CameraControl const&) = delete;

    bool is_available(TFV_Id camera_id);
    bool acquire(TFV_Id camera_id);
    bool is_open(TFV_Id camera_id);
    // dec user count by 1, only close cam if unused
    void safe_release(TFV_Id camera_id);
    // erase users of cam and close cam
    void release(TFV_Id camera_id);
    // also clear usermap
    void release_all(void);

    bool get_frame_size(TFV_Id camera_id, int& rows, int& columns);
    bool get_frame(TFV_Id camera_id, TFV_ImageData* frame);

private:
    using CamId = TFV_Id;
    using CamMap = std::map<CamId, Camera*>;
    CamMap camera_map_;

    using Users = TFV_Id;
    using UserCount = std::map<CamId, Users>;
    UserCount camera_user_count_;

    TFV_Id max_users_per_cam_;
};
};

#endif /* CAMERACONTROL_H */
