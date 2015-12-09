/// \file gesture.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Declaration of the Module \c Gesture.
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

#ifndef GESTURE_H
#define GESTURE_H

#include "module.hh"
#include "detect.hh"

namespace tv {

class Gesture : public Module {
private:
    uint8_t const min_saturation{0};    ///< Lower limit
    uint8_t const max_saturation{255};  ///< Upper limit

    uint8_t const min_value{0};    ///< Lower limit
    uint8_t const max_value{255};  ///< Upper limit

    uint8_t const min_hue{3};   ///< Lower limit
    uint8_t const max_hue{33};  ///< Upper limit

    enum class State : uint8_t { Initial, Detect, Track, Match };

    State state_{State::Detect};

    Hand hand_;
    Detect detect_;
    ImageHeader ref_header_;

public:
    Gesture(Environment const& envir) : Module("gesture", envir) {
        cv::namedWindow("Canny");
        cv::namedWindow("HSV");

        register_parameter("fg-threshold", 0, 255, 50);
        register_parameter("bg-history", 0, 150, 20);
        register_parameter("min-hand-size", 0, 76800, 200);  // max: 320x240
    }

    ~Gesture(void) override = default;

public:
    /// Execute this module. Parameters 3 and 4 are ignored here because this
    /// module does not manipulate the image.
    void execute(tv::ImageHeader const& header, ImageData const* data,
                 tv::ImageHeader const&, ImageData*) override;

    /// Declare the expected colorspace.
    /// \return ColorSpace::BGR888
    ColorSpace input_format(void) const override { return ColorSpace::BGR888; }

    /// This module does not modify the image.
    /// \return false.
    bool outputs_image(void) const override final { return false; }

    /// Declare that this module does not generate a result.
    /// \return false.
    bool produces_result(void) const override final { return false; }

    void value_changed(std::string const& parameter, int32_t value);
};
}

DECLARE_VISION_MODULE(Gesture)

#endif
