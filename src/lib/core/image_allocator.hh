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

#ifndef IMAGE_ALLOCATOR_H
#define IMAGE_ALLOCATOR_H

#include "image.hh"

namespace tfv {

class ImageAllocator {
private:
    Image image_;
    size_t image_init_bytesize_{0};

    size_t max_size_{
        0};  ///< Optional size limit if known during initialization.

public:
    ImageAllocator(void) = default;
    explicit ImageAllocator(size_t known_max_size)
        : max_size_{known_max_size} {}

    ImageAllocator(ImageAllocator const&) = delete;
    ImageAllocator& operator=(ImageAllocator const&) = delete;

    ~ImageAllocator(void);

    bool allocate(uint16_t width, uint16_t height, size_t bytesize,
                  ColorSpace format);

    Image& image(void) { return image_; }
};
}

#endif
