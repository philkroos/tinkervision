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

#include "opencv_camera.hh"

#define __TFV_CV_CAMERA_CHANNELS 3

tfv::OpenCvUSBCamera::OpenCvUSBCamera(TFV_Id camera_id)
    : Camera(camera_id, __TFV_CV_CAMERA_CHANNELS) {}

#undef __TFV_CV_CAMERA_CHANNELS

bool tfv::OpenCvUSBCamera::open(void) {

    camera_ = new cv::VideoCapture(camera_id_);

    auto result = is_open();
    if (not result) {
        stop();
    }

    return result;
}

bool tfv::OpenCvUSBCamera::is_open(void) {
    return camera_ and camera_->isOpened();
}

void tfv::OpenCvUSBCamera::close(void) {
    if (camera_) {
        camera_->release();
        delete camera_;
    }
    camera_ = nullptr;
}

bool tfv::OpenCvUSBCamera::get_properties(int& height, int& width,
                                          int& channels) {

    bool known = false;
    if (width_ == -1 or height_ == -1) {
        if (is_open()) {
            width_ = static_cast<int>(camera_->get(CV_CAP_PROP_FRAME_WIDTH));
            height_ = static_cast<int>(camera_->get(CV_CAP_PROP_FRAME_HEIGHT));
        }
    }
    height = height_;
    width = width_;
    channels = channels_;

    return known;
}

bool tfv::OpenCvUSBCamera::retrieve_frame(TFV_ImageData* frame) {
    auto result = is_open();

    if (result) {
        camera_->grab();

        cv::Mat container(height_, width_, flag_, frame);

        // can't fill container directly; retrieve initializes a new data block
        cv::Mat tmp;
        result = camera_->retrieve(tmp);

        if (result) {
            tmp.copyTo(container);
        }
    }
    return result;
}
