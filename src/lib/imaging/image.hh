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

#include <chrono>  // timestamp

#include "tinkervision_defines.h"

namespace tv {

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
 * This equals YUV422.
 *
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
    /*YVYU,*/
    YV12,
    BGR888,
    RGB888,
    GRAY
};

using Clock = std::chrono::steady_clock;
using Timestamp = Clock::time_point;
using ImageData = TV_ImageData;

struct ImageHeader {
    uint16_t width = 0;
    uint16_t height = 0;
    size_t bytesize = 0;
    Timestamp timestamp;
    ColorSpace format = ColorSpace::INVALID;
    operator bool(void) const {
        return width > 0 and height > 0 and bytesize > 0 and
               format != tv::ColorSpace::INVALID;
    }
};

bool operator==(ImageHeader const& lhs, ImageHeader const& rhs);
bool operator!=(ImageHeader const& lhs, ImageHeader const& rhs);

struct Image {
    ImageHeader header;
    ImageData* data = nullptr;
};

class ImageAllocator {
private:
    Image image_;
    size_t image_init_bytesize_{0};
    bool using_foreign_data_{false};

    size_t max_size_{0};  ///< Optional size limit if known at initialization.

    void _free_image(void);

public:
    ImageAllocator(void) = default;
    explicit ImageAllocator(size_t known_max_size)
        : max_size_{known_max_size} {}

    ImageAllocator(ImageAllocator const&) = delete;
    ImageAllocator& operator=(ImageAllocator const&) = delete;

    ~ImageAllocator(void);

    bool allocate(uint16_t width, uint16_t height, size_t bytesize,
                  ColorSpace format, bool foreign_data);

    bool allocate(ImageHeader const& header, bool foreign_data) {
        return allocate(header.width, header.height, header.bytesize,
                        header.format, foreign_data);
    }

    void set_from_image(Image const& image);

    void copy_data(ImageData const* data, size_t size);

    Image& image(void) { return image_; }
    Image const& operator()(void) const { return image_; }
};
}

#endif
