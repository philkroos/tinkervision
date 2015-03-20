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

#include <sys/stat.h>  // stat, for fast device_exists()
#include <string>

#include "camera.hh"

namespace tfv {

class CameraControl {
public:
    CameraControl(void) = default;
    ~CameraControl(void);

    CameraControl(CameraControl const&) = delete;
    CameraControl& operator=(CameraControl const&) = delete;

    inline bool device_exists(int number) {
        struct stat buffer;
        return (stat(std::string("/dev/video" + std::to_string(number)).c_str(),
                     &buffer) == 0);
    }

    bool is_available(TFV_Id camera_id);
    bool acquire(TFV_Id camera_id);
    bool is_open(TFV_Id camera_id);
    // erase users of cam and close cam
    void release(TFV_Id camera_id);
    // also clear usermap
    void release_all(void);

    bool get_properties(TFV_Id camera_id, int& height, int& width,
                        int& channels);
    bool get_frame(TFV_Id camera_id, TFV_ImageData* frame);

private:
    using CamId = TFV_Id;
    using CamMap = std::map<CamId, Camera*>;
    CamMap camera_map_;
};
};

#endif /* CAMERACONTROL_H */
