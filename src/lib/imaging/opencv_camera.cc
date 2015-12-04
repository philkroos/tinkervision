/// \file opencv_camera.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Definition of a camera using the OpenCV-interface.
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

#ifdef WITH_OPENCV_CAM

#include "opencv_camera.hh"
#include "logger.hh"

tv::OpenCvUSBCamera::OpenCvUSBCamera(uint8_t camera_id) : Camera(camera_id) {}

bool tv::OpenCvUSBCamera::open_device(void) {

    assert(not camera_);
    if (camera_) {
        return false;
    }
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

bool tv::OpenCvUSBCamera::open_device(uint16_t width, uint16_t height) {

    assert(not camera_);
    if (camera_) {
        return false;
    }
    camera_ = new cv::VideoCapture(camera_id_);

    auto result = is_open();
    if (not result) {
        close();
    } else {
        if (not camera_->set(CV_CAP_PROP_FRAME_WIDTH,
                             static_cast<double>(width))) {
            LogWarning("OPENCV_CAM", "Could not set framewidth to ", width);
        }
        if (not camera_->set(CV_CAP_PROP_FRAME_HEIGHT,
                             static_cast<double>(height))) {
            LogWarning("OPENCV_CAM", "Could not set frameheight to ", height);
        }
        _retrieve_properties();
    }

    return result;
}

bool tv::OpenCvUSBCamera::is_open(void) const {
    return camera_ and camera_->isOpened();
}

void tv::OpenCvUSBCamera::close(void) {
    if (camera_) {
        camera_->release();
        delete camera_;
    }

    camera_ = nullptr;
}

void tv::OpenCvUSBCamera::retrieve_properties(uint16_t& width, uint16_t& height,
                                              size_t& frame_bytesize) {

    _retrieve_properties();
    height = frame_height_;
    width = frame_width_;
    frame_bytesize = frame_bytesize_;
}

bool tv::OpenCvUSBCamera::retrieve_frame(tv::ImageData** data) {

    auto result = is_open();
    if (result) {

        _retrieve_properties();
        camera_->grab();

        result = camera_->retrieve(container_);

        if (result) {

            *data = container_.data;
        }
    }

    return result;
}

void tv::OpenCvUSBCamera::_retrieve_properties(void) {

    if (not frame_width_ and is_open()) {
        frame_width_ = static_cast<int>(camera_->get(CV_CAP_PROP_FRAME_WIDTH));
        frame_height_ =
            static_cast<int>(camera_->get(CV_CAP_PROP_FRAME_HEIGHT));
        frame_bytesize_ = frame_width_ * frame_height_ * 3;  // RGB
    }
}

#endif
