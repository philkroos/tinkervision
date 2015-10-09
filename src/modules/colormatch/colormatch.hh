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

#include "tinkervision/tv_module.hh"

namespace tv {

class Colormatch : public Analyzer {
private:
    TV_Byte const min_saturation{0};
    TV_Byte const max_saturation{255};
    TV_Byte const min_value{0};
    TV_Byte const max_value{255};

    TV_Byte const min_hue{0};
    TV_Byte const max_hue{180};

    // configurable values
    TV_Byte user_min_hue;         ///< The used minimum hue
    TV_Byte user_max_hue;         ///< The used maximum hue
    TV_Byte user_min_value;       ///< The used minimum value
    TV_Byte user_max_value;       ///< The used maximum value
    TV_Byte user_min_saturation;  ///< The used minimum saturation
    TV_Byte user_max_saturation;  ///< The used maximum saturation

    Result result_;

    TV_Context context;
    bool in_range(TV_Word value, TV_Word low, TV_Word high) {
        return value >= low and value <= high;
    }

public:
    Colormatch(void)
        : Analyzer("Colormatch"),
          user_min_hue(min_hue),
          user_max_hue(max_hue),
          user_min_value(min_value),
          user_max_value(max_value),
          user_min_saturation(min_saturation),
          user_max_saturation(max_saturation) {}

    ~Colormatch(void) override = default;

    void execute(tv::ImageHeader const& header, ImageData const* data) override;
    ColorSpace expected_format(void) const override {
        return ColorSpace::BGR888;
    }

    virtual void parameter_list(std::vector<std::string>& parameters) {
        parameters.push_back("min-hue");
        parameters.push_back("max-hue");
        parameters.push_back("min-value");
        parameters.push_back("max-value");
        parameters.push_back("min-saturation");
        parameters.push_back("max-saturation");
    }

    bool has_parameter(std::string const& parameter) const override {
        return parameter == "min-hue" or parameter == "min-value" or
               parameter == "min-saturation" or parameter == "max-hue" or
               parameter == "max-value" or parameter == "max-saturation";
    }

    bool set(std::string const& parameter, TV_Word value) override {
        if (parameter == "min-hue" and in_range(value, min_hue, max_hue)) {
            user_min_hue = static_cast<TV_Byte>(value);
            return true;
        } else if (parameter == "max-hue" and
                   in_range(value, min_hue, max_hue)) {
            user_max_hue = static_cast<TV_Byte>(value);
            return true;
        } else if (parameter == "min-saturation" and
                   in_range(value, min_saturation, max_saturation)) {
            user_min_saturation = static_cast<TV_Byte>(value);
            return true;
        } else if (parameter == "max-saturation" and
                   in_range(value, min_saturation, max_saturation)) {
            user_max_saturation = static_cast<TV_Byte>(value);
            return true;
        } else if (parameter == "min-value" and
                   in_range(value, min_value, max_value)) {
            user_min_value = static_cast<TV_Byte>(value);
            return true;
        } else if (parameter == "max-value" and
                   in_range(value, min_value, max_value)) {
            user_max_value = static_cast<TV_Byte>(value);
            return true;
        }

        return false;
    }

    TV_Word get(std::string const& parameter) override {
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
