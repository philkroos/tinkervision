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

#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>

#include "tinkervision_defines.h"
#include "image.hh"

namespace tfv {

class Camera {
public:
    virtual ~Camera(void) = default;

    void stop(void);
    bool get_frame(Image& frame);
    bool get_properties(uint_fast16_t& height, uint_fast16_t& width,
                        size_t& framebytesize);

    virtual bool open(void) = 0;
    virtual bool is_open(void) const = 0;

    virtual ColorSpace image_format(void) const = 0;

protected:
    explicit Camera(TFV_Id camera_id);
    Camera(TFV_Id camera_id, uint_fast16_t framewidth, uint_fast16_t frameheight);
    TFV_Id camera_id_;

    bool requested_settings(void) const { return requested_width_ != 0; }

    uint_fast16_t requested_framewidth(void) const { return requested_width_; }

    uint_fast16_t requested_frameheight(void) const { return requested_height_; }

    // These are Template Methods, see implementation
    // of the corresponding get_ methods.
    virtual bool retrieve_frame(Image& frame) = 0;
    virtual void retrieve_properties(uint_fast16_t& width, uint_fast16_t& height,
                                     size_t& framebytesize) = 0;
    virtual void close(void) = 0;

private:
    bool active_{true};
    uint_fast16_t requested_width_{0};
    uint_fast16_t requested_height_{0};
};
}

#endif /* CAMERA_H */
