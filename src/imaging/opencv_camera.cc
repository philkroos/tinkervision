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

tfv::OpenCvUSBCamera::OpenCvUSBCamera(TFV_Id camera_id) : Camera(camera_id) {}

bool tfv::OpenCvUSBCamera::open(void) {

    camera_ = new cv::VideoCapture(camera_id_);

    auto result = is_open();
    if (not result) {
        close();
    } else {
        if (not frame_width_) {
            _retrieve_properties();
        }
    }

    return result;
}

bool tfv::OpenCvUSBCamera::is_open(void) const {
    return camera_ and camera_->isOpened();
}

void tfv::OpenCvUSBCamera::close(void) {
    if (camera_) {
        camera_->release();
        delete camera_;
    }

    camera_ = nullptr;
}

void tfv::OpenCvUSBCamera::retrieve_properties(uint16_t& width,
                                               uint16_t& height,
                                               size_t& frame_bytesize) {

    _retrieve_properties();
    height = frame_height_;
    width = frame_width_;
    frame_bytesize = frame_bytesize_;
}

bool tfv::OpenCvUSBCamera::retrieve_frame(tfv::Image& image) {
    image.data = nullptr;
    image.bytesize = 0;

    auto result = is_open();
    if (result) {

        _retrieve_properties();
        camera_->grab();

        result = camera_->retrieve(container_);

        if (result) {
            // assert:
            // container_.cols * container_.elemSize() == bytesize

            image.data = container_.data;
            image.bytesize = frame_bytesize_;
            image.width = frame_width_;
            image.height = frame_height_;
        }
    }

    return result;
}

void tfv::OpenCvUSBCamera::_retrieve_properties(void) {

    if (not frame_width_ and is_open()) {
        frame_width_ = static_cast<int>(camera_->get(CV_CAP_PROP_FRAME_WIDTH));
        frame_height_ =
            static_cast<int>(camera_->get(CV_CAP_PROP_FRAME_HEIGHT));
        frame_bytesize_ = frame_width_ * frame_height_ * 3;  // RGB
    }
}
