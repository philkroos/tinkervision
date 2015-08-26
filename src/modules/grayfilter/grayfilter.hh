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

#ifndef GRAYFILTER_H
#define GRAYFILTER_H

#include <opencv2/opencv.hpp>
#ifdef DEBUG  // need to link with libtinkervision_dbg
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#endif

#include "tv_module.hh"

namespace tfv {

struct Grayfilter : public TVModule {
private:
    TFV_Context context;

public:
    Grayfilter(void) : TVModule("Grayfilter") {
#ifdef DEBUG
        cv::namedWindow("Grayfilter");
#endif
    }

    ~Grayfilter(void) override = default;

    void execute_modifying(tfv::ImageData* data, size_t width,
                           size_t height) override;

    ColorSpace expected_format(void) const override {
        return ColorSpace::BGR888;
    }

    bool modifies_image(void) const override { return true; }
};
}

DECLARE_VISION_MODULE(Grayfilter)

#endif
