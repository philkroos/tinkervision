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

#include <thread>
#include <mutex>

#include <opencv2/opencv.hpp>

#include "tinkervision_defines.h"

namespace tfv {

class Camera {
public:
    virtual ~Camera(void) {
        active_ = false;
        if (grabber_thread_.joinable()) {
            grabber_thread_.join();
        }
    }

    void stop(void);
    bool get_frame(TFV_ImageData* frame);

    virtual bool open(void) = 0;
    virtual bool is_open(void) = 0;
    virtual bool get_properties(int& height, int& width, int& channels) = 0;

protected:
    Camera(TFV_Id camera_id, int latency, int channels);

    TFV_Id camera_id_;
    int width_ = -1;
    int height_ = -1;
    int channels_ = -1;
    int latency_ = 0;  // milliseconds b/w grabbing. If <0, single-threaded.

    virtual void grab_frame(void) = 0;

    // These are Template Methods (Design Pattern)
    virtual bool retrieve_frame(TFV_ImageData* frame) = 0;
    virtual void close(void) = 0;

private:
    std::thread grabber_thread_;  // grab and retrieve frames in seperate thread
    std::mutex mutex_;            // either grab or retrieve
    bool active_ = true;

    void grab_loop(void);
};

class CameraUsbOpenCv : public Camera {

public:
    CameraUsbOpenCv(TFV_Id camera_id, TFV_Byte channels, TFV_Int latency);
    virtual ~CameraUsbOpenCv(void);

    virtual bool open(void);
    virtual bool is_open(void);
    virtual bool get_properties(int& height, int& width, int& channels);

protected:
    virtual void grab_frame(void);
    virtual bool retrieve_frame(TFV_ImageData* frame);
    virtual void close(void);

private:
    cv::VideoCapture* camera_ = nullptr;
    TFV_Int flag_ = CV_8UC3;  // default: color
};
};
