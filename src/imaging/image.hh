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

#ifndef IMAGING_H
#define IMAGING_H

#include <cstddef>
#include <iostream>

namespace tfv {

/**
 * Supported image formats. The value range per entry is 0-255 for each format,
 * but the number of bytes per pixel differs.
 * - NONE: Can be used by modules that don't want to process images.
 * - INVALID: This would be an error.
 * - YUYV: packed Y'CbCr data, i.e. Y,V and U are stored in the same
 * datablock, structured like:
 * Y00 U00 Y01 V00 Y02 U01 Y03 V01 Y04 ...
 * Y10 U10 Y11 V10 Y12 U11 Y13 V11 Y14 ...
 * ...
 * For each two Y-values there is one U and one V and one pixel is represented
 * by two bytes. So the number of bytes per row equals width*2.
 * - YV12: planar Y'CbCr data, i.e. Y,U and V are stored in consecutive
 * datablocks, structured like:
 * Y00 Y01 Y02 ...
 * Y10 Y11 Y12 ...
 * ...
 * U00 U01 U02 ...
 * ...
 * V00 V01 V02 ...
 * ...
 * where the count(U) = count(V) = count(Y)/4, i.e. count(Y) = width * height
 * and the resolution of U and V is halfed vertically and horizontally.
 * - RGB888: One pixel p = 3 byte where p(1) = R, p(2) = G, p(3) = B
 * So the size of one image is height * width * 3.
 * - BGR888: The same as RGB888 but reordered.
 */
enum class ColorSpace : char {
    NONE,
    INVALID,
    YUYV,
    /*YVYU,*/ YV12,
    BGR888,
    RGB888
};

std::ostream& operator<<(std::ostream& ost, ColorSpace const& format);

using ImageData = unsigned char;
using Timestamp = unsigned long;

struct Image {
    size_t width = 0;
    size_t height = 0;
    size_t bytesize = 0;
    ImageData* data = nullptr;
    Timestamp timestamp;
    ColorSpace format = ColorSpace::INVALID;
};
}

#endif /* IMAGING_H */
