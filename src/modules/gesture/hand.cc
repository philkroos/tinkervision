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

#include "hand.hh"

std::ostream& operator<<(std::ostream& o, Hand const& hand) {
    o << hand.x << "," << hand.y << "-" << hand.width << "," << hand.height
      << std::endl;
    return o;
}

void bgr_average(Hand const& hand, tv::ImageData const* data, size_t dataw,
                 uint8_t& b, uint8_t& g, uint8_t& r) {

    uint32_t b32 = 0, g32 = 0, r32 = 0;
    auto const width = 3 * dataw;
    auto const handx = 3 * hand.x;
    for (size_t i = hand.y; i < hand.y + hand.height; ++i) {
        auto ptr = data + width * i + handx;

        for (size_t j = 0; j < hand.width; ++j) {
            b32 += *ptr++;
            g32 += *ptr++;
            r32 += *ptr++;
        }
    }
    auto const size = hand.width * hand.height;
    assert(size > 0);
    b = static_cast<uint8_t>(b32 / size);
    g = static_cast<uint8_t>(g32 / size);
    r = static_cast<uint8_t>(r32 / size);
}

void bgr_average(std::vector<Pixel> const& pixels, tv::ImageData const* data,
                 uint8_t& b, uint8_t& g, uint8_t& r) {

    assert(pixels.size());

    uint32_t b32 = 0, g32 = 0, r32 = 0;
    auto ptr = data + pixels[0].idx * 3;
    for (size_t i = 0; i < pixels.size(); ++i, ptr = data + pixels[i].idx * 3) {
        b32 += *ptr++;
        g32 += *ptr++;
        r32 += *ptr;
    }

    auto const size = pixels.size();
    b = static_cast<uint8_t>(b32 / size);
    g = static_cast<uint8_t>(g32 / size);
    r = static_cast<uint8_t>(r32 / size);
}
