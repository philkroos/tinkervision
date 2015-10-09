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

#include "image.hh"

#include <algorithm>  // copy_n
#include <cassert>
#include <iostream>

#include "logger.hh"

bool tv::operator==(tv::ImageHeader const& lhs, tv::ImageHeader const& rhs) {
    return lhs.width == rhs.width and lhs.height == rhs.height and
           lhs.bytesize == rhs.bytesize and lhs.format == rhs.format;
}
bool tv::operator!=(tv::ImageHeader const& lhs, tv::ImageHeader const& rhs) {
    return not(lhs == rhs);
}

tv::ImageAllocator::~ImageAllocator(void) { _free_image(); }

bool tv::ImageAllocator::allocate(uint16_t width, uint16_t height,
                                  size_t bytesize, ColorSpace format,
                                  bool foreign_data) {

    if (max_size_ > 0 and bytesize > max_size_) {
        LogError("ImageAllocator", bytesize, " bytes requested. Allowed: ",
                 max_size_);
        return false;
    }

    // only reallocate memory if necessary
    if (bytesize != image_init_bytesize_) {
        _free_image();
    }

    if (not image_.data) {
        image_init_bytesize_ = bytesize;
        if (not foreign_data) {
            image_.data = new TV_ImageData[bytesize];
        }
    }

    using_foreign_data_ = foreign_data;

    image_.header.bytesize = bytesize;
    image_.header.width = width;
    image_.header.height = height;
    image_.header.format = format;

    return true;
}

void tv::ImageAllocator::set_from_image(Image const& image) {
    _free_image();

    image_ = image;
    using_foreign_data_ = true;
}

void tv::ImageAllocator::copy_data(ImageData const* data, size_t size) {
    assert(not using_foreign_data_);
    assert(image_.data);
    assert(image_.header.bytesize == size);
    //_free_image();

    std::copy_n(data, image_.header.bytesize, image_.data);
}

void tv::ImageAllocator::_free_image(void) {
    if (image_.data and not using_foreign_data_) {
        delete[] image_.data;
    }
    image_.header.bytesize = 0;
    image_.header.format = ColorSpace::INVALID;
    image_.data = nullptr;
}
