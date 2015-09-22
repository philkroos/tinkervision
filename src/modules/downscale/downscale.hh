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

#ifndef DOWNSCALE_H
#define DOWNSCALE_H

#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include "tv_module.hh"

namespace tfv {

struct Downscale : public Modifier {
private:
    uint8_t factor_{1};  ///< 1 is half size, 2 quarter, ...
    uint8_t max_factor_{5};

public:
    Downscale(void)
        : Modifier("Downscale") { /* cv::namedWindow("Downscale"); */
    }

    ~Downscale(void) override final = default;

    void get_header(ImageHeader const& ref, ImageHeader& output) override final;

    void execute(tfv::ImageHeader const& header, tfv::ImageData const* data,
                 tfv::Image& output) override final;

    ColorSpace expected_format(void) const override final {
        return ColorSpace::BGR888;
    }

    /**
     * If factor is zero, no need to be executed.
     */
    bool running(void) const noexcept override final { return factor_ > 0; }

    bool has_parameter(std::string const& parameter) const override final {
        return parameter == "factor";
    }

    bool set(std::string const& parameter, TFV_Word value) override final {
        if ((parameter != "factor") or (value < 0) or (value > max_factor_)) {
            return false;
        }

        factor_ = static_cast<uint8_t>(value);
        return true;
    }

    TFV_Word get(std::string const& parameter) override final {
        if (parameter == "factor") {
            return static_cast<TFV_Word>(factor_);
        }
        return 0;
    }
};
}

DECLARE_VISION_MODULE(Downscale)

#endif
