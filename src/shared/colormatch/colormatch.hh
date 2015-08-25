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

#ifndef COLORMATCH_H
#define COLORMATCH_H

#ifdef DEBUG
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#endif

#include "tv_module.hh"

namespace tfv {

class Colormatch : public TVModule {
private:
    TFV_Byte const min_saturation{0};
    TFV_Byte const max_saturation{255};
    TFV_Byte const min_value{0};
    TFV_Byte const max_value{255};

    TFV_Byte const min_hue{0};
    TFV_Byte const max_hue{180};

    // configurable values
    TFV_Byte user_min_hue;         ///< The used minimum hue
    TFV_Byte user_max_hue;         ///< The used maximum hue
    TFV_Byte user_min_value;       ///< The used minimum value
    TFV_Byte user_max_value;       ///< The used maximum value
    TFV_Byte user_min_saturation;  ///< The used minimum saturation
    TFV_Byte user_max_saturation;  ///< The used maximum saturation

    PointResult result_;

    TFV_Context context;
    bool in_range(TFV_Word value, TFV_Word low, TFV_Word high) {
        return value >= low and value <= high;
    }

public:
    Colormatch(void)
        : TVModule("Colormatch"),
          user_min_hue(min_hue),
          user_max_hue(max_hue),
          user_min_value(min_value),
          user_max_value(max_value),
          user_min_saturation(min_saturation),
          user_max_saturation(max_saturation) {
#ifdef DEBUG
        cv::namedWindow("Colormatch");
#endif
    }

    ~Colormatch(void) override = default;
    void execute(tfv::Image const& image) override;
    ColorSpace expected_format(void) const override {
        return ColorSpace::BGR888;
    }

    bool has_parameter(std::string const& parameter) const override {
        return parameter == "min-hue" or parameter == "min-value" or
               parameter == "min-saturation" or parameter == "max-hue" or
               parameter == "max-value" or parameter == "max-saturation";
    }

    bool set(std::string const& parameter, TFV_Word value) override {
        if (parameter == "min-hue" and in_range(value, min_hue, max_hue)) {
            user_min_hue = static_cast<TFV_Byte>(value);
            return true;
        } else if (parameter == "max-hue" and
                   in_range(value, min_hue, max_hue)) {
            user_max_hue = static_cast<TFV_Byte>(value);
            return true;
        } else if (parameter == "min-saturation" and
                   in_range(value, min_saturation, max_saturation)) {
            user_min_saturation = static_cast<TFV_Byte>(value);
            return true;
        } else if (parameter == "max-saturation" and
                   in_range(value, min_saturation, max_saturation)) {
            user_max_saturation = static_cast<TFV_Byte>(value);
            return true;
        } else if (parameter == "min-value" and
                   in_range(value, min_value, max_value)) {
            user_min_value = static_cast<TFV_Byte>(value);
            return true;
        } else if (parameter == "max-value" and
                   in_range(value, min_value, max_value)) {
            user_max_value = static_cast<TFV_Byte>(value);
            return true;
        }

        return false;
    }

    TFV_Word get(std::string const& parameter) override {
        if (parameter == "min-hue") {
            return user_min_hue;
        } else if (parameter == "max-hue") {
            return user_max_hue;
        } else if (parameter == "min-value") {
            return user_min_value;
        } else if (parameter == "max-value") {
            return user_max_value;
        } else if (parameter == "min-saturation") {
            return user_min_saturation;
        } else if (parameter == "max-saturation") {
            return user_max_saturation;
        }

        assert(false);
        return -1;  // never reached
    }

    Result const* get_result(void) const override { return &result_; }
};
}

DECLARE_VISION_MODULE(Colormatch)

#endif /* COLORMATCH_H */
