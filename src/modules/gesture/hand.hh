/// \file hand.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of \c Hand.
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

#include <iostream>
#include <vector>

#include <tinkervision/image.hh>

#include "pixel.hh"

struct Hand {
    uint16_t x, y;                    ///< Minima
    uint16_t width, height;           ///< Maxima - Minima
    uint16_t centroid_x, centroid_y;  ///< absolute

    uint8_t b, g, r;  ///< Average around centroid
};

std::ostream& operator<<(std::ostream& o, Hand const& hand);

bool make_hand(uint8_t const* frame, uint16_t framewidth,
               std::vector<Pixel> const& pixel, Hand& hand);

void bgr_average(Hand const& hand, tv::ImageData const* data, size_t dataw,
                 uint8_t& b, uint8_t& g, uint8_t& r);

void bgr_average(std::vector<Pixel> const& pixels, tv::ImageData const* data,
                 uint8_t& b, uint8_t& g, uint8_t& r);
