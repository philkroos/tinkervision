/// \file opencv_camera.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of a camera using the OpenCV-interface.
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

#ifndef OPENCV_CAMERA_H
#define OPENCV_CAMERA_H

#ifdef WITH_OPENCV_CAM

#include <opencv2/opencv.hpp>

#include "camera.hh"
#include "image.hh"

namespace tv {

class OpenCvUSBCamera : public Camera {

public:
    explicit OpenCvUSBCamera(uint8_t camera_id);
    ~OpenCvUSBCamera(void) override final { close(); }

    bool open_device(void) override final;
    bool open_device(uint16_t, uint16_t) override final;
    bool is_open(void) const override final;
    ColorSpace image_format(void) const override final {
        return ColorSpace::BGR888;
    }

protected:
    bool retrieve_frame(tv::ImageData** data) override final;
    void retrieve_properties(uint16_t& width, uint16_t& height,
                             size_t& frame_bytesize) override final;
    void close(void) override final;

private:
    cv::VideoCapture* camera_ = nullptr;
    static const int16_t flag_ = CV_8UC3;  // default: color
    cv::Mat container_;

    size_t frame_width_ = 0;     ///< resolution width
    size_t frame_height_ = 0;    ///< resolution height
    size_t frame_bytesize_ = 0;  ///< size of data retrieved per frame

    void _retrieve_properties(void);
};
}

#endif
#endif
