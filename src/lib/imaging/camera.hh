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

namespace tv {

class Camera {
public:
    virtual ~Camera(void) = default;

    void stop(void);
    bool get_frame(Image& frame);
    bool get_properties(uint16_t& height, uint16_t& width,
                        size_t& framebytesize);
    bool open(void);

    ImageHeader frame_header(void) const { return image_.header; }

    virtual bool is_open(void) const = 0;
    virtual ColorSpace image_format(void) const = 0;

protected:
    explicit Camera(TV_Id camera_id);
    Camera(TV_Id camera_id, uint16_t framewidth, uint16_t frameheight);
    TV_Id camera_id_;

    bool requested_settings(void) const { return requested_width_ != 0; }

    uint16_t requested_framewidth(void) const { return requested_width_; }

    uint16_t requested_frameheight(void) const { return requested_height_; }

    // These are Template Methods, see implementation
    // of the corresponding get_ methods.
    virtual bool open_device(void) = 0;
    virtual bool retrieve_frame(tv::ImageData** data) = 0;
    virtual void retrieve_properties(uint16_t& width, uint16_t& height,
                                     size_t& framebytesize) = 0;
    virtual void close(void) = 0;

private:
    bool active_{true};
    uint16_t requested_width_{0};
    uint16_t requested_height_{0};

    Image image_{};  ///< Image container, data filled by subclass
};
}

#endif /* CAMERA_H */
