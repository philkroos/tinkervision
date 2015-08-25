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

#ifndef MOTIONDETECT_H
#define MOTIONDETECT_H

#include <opencv2/opencv.hpp>

#include "tv_module.hh"

namespace tfv {

struct Motiondetect : public TVModule {
private:
    // see <opencv-source>/modules/video/src/bgfg_gaussmix2.cpp
    int history_{20};      // default constructor: 500
    float threshold_{16};  // default constructor: 16
    bool shadows_{false};  // default constructor: true
    cv::BackgroundSubtractorMOG2 background_subtractor_{history_, threshold_,
                                                        shadows_};
    const size_t min_contour_count_{10};  ///< ignoring 'small' motions
    int framecounter_{0};                 ///< ignoring the first frames

    bool results_{false};  ///< true if motion detected in last frame

    RectangleResult rect_around_motion_;

public:
    Motiondetect(void) : TVModule{"Motiondetect"} {}

    ~Motiondetect(void) override = default;
    void execute(tfv::ImageData const* data, size_t width,
                 size_t height) override;

    ColorSpace expected_format(void) const override {
        return ColorSpace::BGR888;
    }

    Result const* get_result(void) const override {
        return &rect_around_motion_;
    }
};

DECLARE_VISION_MODULE(Motiondetect)
}
#endif /* MOTIONDETECT_H */
