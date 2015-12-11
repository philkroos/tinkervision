/// \file dummy.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of the module \c Blocking.
/// A blocking module to improve the library on high-latency libraries.
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

#pragma once

#include "module.hh"

namespace tv {
class Blocking : public Module {
public:
    Blocking(Environment const& envir) : Module("blocking", envir) {}

    /// Returning None prevents this module from being executed.
    ColorSpace input_format(void) const override { return ColorSpace::NONE; }

    bool outputs_image(void) const override { return false; }

    bool produces_result(void) const override { return false; }

    void execute(tv::ImageHeader const&, tv::ImageData const*,
                 tv::ImageHeader const&, tv::ImageData*) override final {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
};
}

DECLARE_VISION_MODULE(Blocking)
