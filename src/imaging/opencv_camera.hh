/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014-2015 philipp.kroos@fh-bielefeld.de

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

#ifndef OPENCV_CAMERA_H
#define OPENCV_CAMERA_H

#include <opencv2/opencv.hpp>

#include "camera.hh"
#include "image.hh"

namespace tfv {

class OpenCvUSBCamera : public Camera {

public:
    explicit OpenCvUSBCamera(TFV_Id camera_id);
    virtual ~OpenCvUSBCamera(void) { close(); }

    virtual bool open(void);
    virtual bool is_open(void) const;
    virtual ColorSpace image_format(void) const { return ColorSpace::BGR888; }

protected:
    virtual bool retrieve_frame(tfv::Image& frame);
    virtual void retrieve_properties(size_t& width, size_t& height,
                                     size_t& frame_bytesize);
    virtual void close(void);

private:
    cv::VideoCapture* camera_ = nullptr;
    static const TFV_Int flag_ = CV_8UC3;  // default: color
    cv::Mat container_;

    size_t frame_width_ = 0;     ///< resolution width
    size_t frame_height_ = 0;    ///< resolution height
    size_t frame_bytesize_ = 0;  ///< size of data retrieved per frame

    void _retrieve_properties(void);
};
}

#endif /* OPENCV_CAMERA_H */
