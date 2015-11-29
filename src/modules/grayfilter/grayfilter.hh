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

struct Grayfilter : public Module {
public:
    Grayfilter(Environment const& envir) : Module("grayfilter", envir) {}

    ~Grayfilter(void) override = default;

protected:
    void execute(tv::ImageHeader const& image, tv::ImageData const* data,
                 tv::ImageHeader const& output_header,
                 tv::ImageData* output_data) override final;

    ColorSpace input_format(void) const override { return ColorSpace::BGR888; }

    ImageHeader get_output_image_header(ImageHeader const& ref) override final;

    bool outputs_image(void) const override final { return true; }

    bool produces_result(void) const override final { return false; }
};
}

DECLARE_VISION_MODULE(Grayfilter)

#endif
