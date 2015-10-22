/// \file Motiondetect.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of the module \c Motiondetect.
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

#ifndef MOTIONDETECT_H
#define MOTIONDETECT_H

#include <opencv2/opencv.hpp>

#include "tinkervision/module.hh"

namespace tv {

struct Motiondetect : public Analyzer {
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

    Result rect_around_motion_;

public:
    Motiondetect(void) : Analyzer{"Motiondetect"} {}

    ~Motiondetect(void) override = default;
    void execute(tv::ImageHeader const& header, ImageData const* data) override;

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
