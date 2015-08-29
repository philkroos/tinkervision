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

#include "image_allocator.hh"

#include "logger.hh"

tfv::ImageAllocator::~ImageAllocator(void) {
    if (image_.data) {
        delete[] image_.data;
    }
    image_.bytesize = 0;
    image_.format = ColorSpace::INVALID;
}

bool tfv::ImageAllocator::allocate(uint16_t width, uint16_t height,
                                   size_t bytesize, ColorSpace format) {

    if (max_size_ > 0 and bytesize > max_size_) {
        LogError("ImageAllocator", bytesize, " bytes requested. Allowed: ",
                 max_size_);
        return false;
    }

    if (image_.data and bytesize > image_init_bytesize_) {
        delete[] image_.data;
    }

    if (not image_.data) {
        image_init_bytesize_ = bytesize;
        image_.data = new TFV_ImageData[bytesize];
    }

    image_.bytesize = bytesize;
    image_.width = width;
    image_.height = height;
    image_.format = format;

    return true;
}
