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

tfv::Camera::Camera(TFV_Id camera_id, int latency)
    : camera_id_(camera_id), latency_(latency) {

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
    std::lock_guard<std::mutex> lock(mutex_);
    return retrieve_frame(frame);
}

void tfv::Camera::stop(void) {
    active_ = false;
    if (grabber_thread_.joinable()) {
        grabber_thread_.join();
    }
    close();
}

tfv::CameraUsbOpenCv::~CameraUsbOpenCv(void) { close(); }

bool tfv::CameraUsbOpenCv::open(void) {
    camera_ = new cv::VideoCapture(camera_id_);

    if (not camera_->isOpened()) {
        close();
        return false;
    }

    return true;
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

bool tfv::CameraUsbOpenCv::get_frame_size(int& rows, int& columns) {
    if (width_ == -1 or height_ == -1) {
        if (not is_open()) {
            return false;
        }
        width_ = static_cast<int>(camera_->get(CV_CAP_PROP_FRAME_WIDTH));
        height_ = static_cast<int>(camera_->get(CV_CAP_PROP_FRAME_HEIGHT));
    }
    rows = height_;
    columns = width_;

    return true;
}

void tfv::CameraUsbOpenCv::grab_frame(void) {
    if (is_open()) {
        camera_->grab();
    }
}

bool tfv::CameraUsbOpenCv::retrieve_frame(TFV_ImageData* frame) {
    if (width_ == -1 or height_ == -1) {
        // expecting the user to ask for these values in advance.
        return false;
    }

    cv::Mat container(width_, height_, CV_8UC1, frame);
    return camera_->retrieve(container);
}
