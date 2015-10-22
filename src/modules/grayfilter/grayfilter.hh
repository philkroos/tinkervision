/// \file grayfilter.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of the module \c Grayfilter.
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

#ifndef GRAYFILTER_H
#define GRAYFILTER_H

#include "tinkervision/module.hh"

namespace tv {

struct Grayfilter : public Modifier {
public:
    Grayfilter(void) : Modifier("Grayfilter") {}

    ~Grayfilter(void) override = default;

    void execute(tv::ImageHeader const& header, tv::ImageData const* data,
                 tv::Image& output) override final;

    ColorSpace expected_format(void) const override {
        return ColorSpace::BGR888;
    }

    void get_header(ImageHeader const& ref, ImageHeader& output) override final;
};
}

DECLARE_VISION_MODULE(Grayfilter)

#endif
