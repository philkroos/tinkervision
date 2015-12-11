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

#include <algorithm>

std::ostream& operator<<(std::ostream& o, Hand const& hand) {
    o << "Rectangle: " << hand.x << "," << hand.y << "-" << hand.width << ","
      << hand.height << std::endl << "Centroid: " << hand.centroid_x << ","
      << hand.centroid_y << std::endl << "Average B,G,R: " << (int)hand.b << ","
      << (int)hand.g << "," << (int)hand.r << std::endl;
    return o;
}

bool make_hand(uint8_t const* frame, uint16_t framewidth,
               std::vector<Pixel> const& pixels, Hand& hand) {
    hand.x = std::numeric_limits<uint16_t>::max();
    hand.y = std::numeric_limits<uint16_t>::max();
    auto maxx = std::numeric_limits<uint16_t>::min();
    auto maxy = std::numeric_limits<uint16_t>::min();

    /// Assuming a hand in upright position and knowing that the list of pixels
    /// starts at the topmost found values, the actual color can be verified by
    /// looking at the first n pixels. If all would be taken into account, most
    /// likely the arm would be considered as well, which probably is not
    /// skin-colored, thereby producing a false negative.
    uint32_t b = 0, g = 0, r = 0;
    uint32_t i = 0;
    while (i++ < 2000 and i < pixels.size()) {
        auto px = frame + pixels[i].idx * 3;
        b += *px++;
        g += *px++;
        r += *px++;
    }

    if (not((r > 95) and (g > 40) and (b > 20) and (r > (g + 15)) and
            (r > (b + 15)))) {
        return false;
    }

    // the above values are considered the actual average skin color
    hand.b = b / (--i);
    hand.g = g / i;
    hand.r = r / i;

    // quick test: look for hand's end by color, allow some errors
    uint8_t missed = 0;
    while (missed < 10 and i < pixels.size()) {  // arbitrary 10
        auto px = frame + pixels[++i].idx * 3;
        b = *px++;
        g = *px++;
        r = *px++;

        missed = (((r > 95) and (g > 40) and (b > 20) and (r > (g + 15)) and
                   (r > (b + 15)))
                      ? 0
                      : missed + 1);
    }

    // get properties of hand: surrounding rectangle, centroid
    uint32_t sumx = 0, sumy = 0;
    for (size_t j = 0; j < i; ++j) {
        auto const& px = pixels[j];

        if (px.x < hand.x) {
            hand.x = px.x;
        } else if (px.x > maxx) {
            maxx = px.x;
        }
        if (px.y < hand.y) {
            hand.y = px.y;
        } else if (px.y > maxy) {
            maxy = px.y;
        }
        sumx += px.x;
        sumy += px.y;
    }
    hand.width = maxx - hand.x;
    hand.height = maxy - hand.y;

    assert(i > 0);

    hand.centroid_x = sumx / i;
    hand.centroid_y = sumy / i;

    assert(hand.width > 0 and hand.height > 0);
    assert(hand.centroid_x < maxx and hand.centroid_x > hand.x);
    assert(hand.centroid_y < maxy and hand.centroid_y > hand.y);

    return true;
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
