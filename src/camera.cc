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

#include <chrono>
#include <iostream>

tfv::Camera::Camera(TFV_Id camera_id, int latency, int channels)
    : camera_id_(camera_id), channels_(channels), latency_(latency) {

    if (latency < 0) {
        latency_ = 0;
    }
    grabber_thread_ = std::thread(&Camera::grab_loop, this);
}

void tfv::Camera::grab_loop(void) {
    while (active_) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            grab_frame();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(latency_));
    }
}

bool tfv::Camera::get_frame(TFV_ImageData* frame) {
    if (not is_open() or width_ == -1 or channels_ == -1) {
        // expecting the user to ask for frame properties in advance.
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        return retrieve_frame(frame);
    }
}

void tfv::Camera::stop(void) {
    active_ = false;
    if (grabber_thread_.joinable()) {
        grabber_thread_.join();
    }
    close();
}

tfv::CameraUsbOpenCv::CameraUsbOpenCv(TFV_Id camera_id, TFV_Byte channels)
    : Camera(camera_id, tfv::CameraUsbOpenCv::latency_,
             static_cast<int>(channels)) {

    if (channels == 1) {
        flag_ = CV_8UC1;
    } else if (channels != 3) {
        channels_ = -1;  // invalid setting
    }
}

tfv::CameraUsbOpenCv::~CameraUsbOpenCv(void) { close(); }

bool tfv::CameraUsbOpenCv::open(void) {

    auto result = false;
    if (channels_ == 1 or channels_ == 3) {
        camera_ = new cv::VideoCapture(camera_id_);
    }

    result = is_open();
    if (not result) {
        close();
    }

    return result;
}

bool tfv::CameraUsbOpenCv::is_open(void) {
    return camera_ and camera_->isOpened();
}

void tfv::CameraUsbOpenCv::close(void) {
    if (camera_) {
        camera_->release();
        delete camera_;
    }
    camera_ = nullptr;
}

bool tfv::CameraUsbOpenCv::get_properties(int& height, int& width,
                                          int& channels) {
    if (width_ == -1 or height_ == -1) {
        if (not is_open()) {
            return false;
        }
        width_ = static_cast<int>(camera_->get(CV_CAP_PROP_FRAME_WIDTH));
        height_ = static_cast<int>(camera_->get(CV_CAP_PROP_FRAME_HEIGHT));
    }
    height = height_;
    width = width_;
    channels = channels_;

    return true;
}

void tfv::CameraUsbOpenCv::grab_frame(void) {
    if (is_open()) {
        camera_->grab();
    }
}

bool tfv::CameraUsbOpenCv::retrieve_frame(TFV_ImageData* frame) {
    cv::Mat container(height_, width_, flag_, frame);

    // can't fill container directly; retrieve initializes a new data block
    cv::Mat tmp;
    auto retrieved = camera_->retrieve(tmp);

#ifdef DEV
    retrieved = retrieved and(tmp.type() == container.type())
        and(tmp.cols == container.cols) and(tmp.rows() == container.rows());
#endif
    if (retrieved) {
      tmp.copyTo(container);
    }

    return retrieved;
}
