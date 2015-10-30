/// \file colormatch.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Declaration of the class \c Colormatch.
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

#ifndef COLORMATCH_H
#define COLORMATCH_H

#include "tinkervision/module.hh"

namespace tv {

class Colormatch : public Analyzer {
private:
    TV_Byte const min_saturation{0};    ///< Lower limit
    TV_Byte const max_saturation{255};  ///< Upper limit

    TV_Byte const min_value{0};    ///< Lower limit
    TV_Byte const max_value{255};  ///< Upper limit

    TV_Byte const min_hue{0};    ///< Lower limit
    TV_Byte const max_hue{180};  ///< Upper limit

    // configurable values
    TV_Byte user_min_hue;         ///< The used minimum hue
    TV_Byte user_max_hue;         ///< The used maximum hue
    TV_Byte user_min_value;       ///< The used minimum value
    TV_Byte user_max_value;       ///< The used maximum value
    TV_Byte user_min_saturation;  ///< The used minimum saturation
    TV_Byte user_max_saturation;  ///< The used maximum saturation

    bool has_result_{false};
    Result result_;

    TV_Context context;

public:
    Colormatch(void)
        : Analyzer("Colormatch"),
          user_min_hue(min_hue),
          user_max_hue(max_hue),
          user_min_value(min_value),
          user_max_value(max_value),
          user_min_saturation(min_saturation),
          user_max_saturation(max_saturation) {

        register_parameter("min-hue", min_hue, max_hue, min_hue);
        register_parameter("max-hue", min_hue, max_hue, min_hue);
        register_parameter("min-value", min_value, max_value, min_value);
        register_parameter("max-value", min_value, max_value, min_value);
        register_parameter("min-saturation", min_saturation, max_saturation,
                           min_saturation);
        register_parameter("max-saturation", min_saturation, max_saturation,
                           min_saturation);
    }

    ~Colormatch(void) override = default;

    void execute(tv::ImageHeader const& header, ImageData const* data) override;
    ColorSpace expected_format(void) const override {
        return ColorSpace::BGR888;
    }

    bool can_have_result(void) const override final { return true; }

protected:
    /// Store the value of changed parameters internally to have faster access.
    /// \param[in] parameter The name of the changed parameter.
    /// \param[in] value New value
    virtual void value_changed(std::string const& parameter,
                               parameter_t value) override final {
        if (parameter == "min-hue") {
            user_min_hue = static_cast<TV_Byte>(value);
        } else if (parameter == "max-hue") {
            user_max_hue = static_cast<TV_Byte>(value);
        } else if (parameter == "min-saturation") {
            user_min_saturation = static_cast<TV_Byte>(value);
        } else if (parameter == "max-saturation") {
            user_max_saturation = static_cast<TV_Byte>(value);
        } else if (parameter == "min-value") {
            user_min_value = static_cast<TV_Byte>(value);
        } else if (parameter == "max-value") {
            user_max_value = static_cast<TV_Byte>(value);
        }
    }

    bool has_result(void) const override final { return has_result_; }
    Result const* get_result(void) const override { return &result_; }
};
}

DECLARE_VISION_MODULE(Colormatch)

#endif
