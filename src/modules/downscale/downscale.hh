/// \file downscale.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of the module \c Downscale.
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

#ifndef DOWNSCALE_H
#define DOWNSCALE_H

#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include "tinkervision/module.hh"

namespace tv {

struct Downscale : public Modifier {
private:
    uint8_t factor_{1};  ///< 1 is half size, 2 quarter, ...
    uint8_t max_factor_{10};

public:
    Downscale(void)
        : Modifier("Downscale") { /* cv::namedWindow("Downscale"); */

        register_parameter("factor", 0, max_factor_, 0);
    }

    ~Downscale(void) override final = default;

    void get_header(ImageHeader const& ref, ImageHeader& output) override final;

    void execute(tv::ImageHeader const& header, tv::ImageData const* data,
                 tv::Image& output) override final;

    ColorSpace expected_format(void) const override final {
        return ColorSpace::BGR888;
    }

    /// If factor is zero, no need to be executed.
    /// \return false if factor <= 0.
    bool running(void) const noexcept override final { return factor_ > 0; }

protected:
    /// Store the value of changed parameters internally to have faster access.
    /// \param[in] parameter The name of the changed parameter.
    /// \param[in] value New value
    virtual void value_changed(std::string const& parameter,
                               parameter_t value) override final {
        factor_ = static_cast<uint8_t>(value);
    }
};
}

DECLARE_VISION_MODULE(Downscale)

#endif
