/// \file image.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Implementation of image.hh declarations.
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

tv::ImageAllocator::ImageAllocator(std::string const& id)
    : ImageAllocator(id, 0) {}

tv::ImageAllocator::ImageAllocator(std::string const& id, size_t known_max_size)
    : id_(id), max_size_{known_max_size} {

    LogDebug("IMAGE_ALLOCATOR", "C'tor for ", id);
}

tv::ImageAllocator::~ImageAllocator(void) {
    LogDebug("IMAGE_ALLOCATOR", "D'tor for ", id_);
    _free_image();
}

bool tv::ImageAllocator::allocate(uint16_t width, uint16_t height,
                                  size_t bytesize, ColorSpace format,
                                  bool foreign_data) {

    LogDebug("IMAGE_ALLOCATOR", "Allocate for ", id_);

    if (max_size_ > 0 and bytesize > max_size_) {
        LogError("ImageAllocator", bytesize, " bytes requested. Allowed: ",
                 max_size_);
        return false;
    }

    /// The currently managed image will be free'd IFF the requested bytesize
    /// differs from the currently allocated or the image manages it's own data
    /// currently.
    if (not foreign_data or bytesize != image_init_bytesize_) {
        _free_image();
    }

    /// A data block for a new image will be then allocated if foreign_data is
    /// false.
    if (not image_.data) {
        image_init_bytesize_ = bytesize;
        if (not foreign_data) {
            image_.data = new uint8_t[bytesize];
            LogDebug("IMAGE_ALLOCATOR", "Allocated data at ",
                     (void*)image_.data, " for ", id_);
        }
    }

    using_foreign_data_ = foreign_data;

    /// The header of image_ is initialized properly.
    image_.header.bytesize = bytesize;
    image_.header.width = width;
    image_.header.height = height;
    image_.header.format = format;

    return true;
}

void tv::ImageAllocator::set_from_image(Image const& image) {
    LogDebug("IMAGE_ALLOCATOR", "Set for ", id_);

    /// The currently managed image is free'd
    _free_image();

    /// and image_ is set to image, creating a shallow copy.
    image_ = image;

    /// After this, the data pointed to by image_ must not be deleted.
    using_foreign_data_ = true;
}

void tv::ImageAllocator::copy_data(ImageData const* data, size_t size) {
    // LogDebug("IMAGE_ALLOCATOR", "Copy for ", id_);

    /// This currently assumes that the method is only called for instances that
    /// are already set to be managing their own data.
    assert(not using_foreign_data_);
    assert(image_.data);
    assert(image_.header.bytesize == size);
    //_free_image();

    /// A bitwise copy of the passed data is created in image_.
    std::copy_n(data, image_.header.bytesize, image_.data);
}

void tv::ImageAllocator::_free_image(void) {
    LogDebug("IMAGE_ALLOCATOR", "FreeImage for ", id_);

    /// Depending on the value of using_foreign_data_, deletes allocated data
    /// block
    if (image_.data and not using_foreign_data_) {
        LogDebug("IMAGE_ALLOCATOR", "Deleting data at ", (void*)image_.data,
                 " for ", id_);
        delete[] image_.data;
    }
    /// and reset the properties of the ImageHeader.
    image_.header.bytesize = 0;
    image_.header.format = ColorSpace::INVALID;
    image_.data = nullptr;
}
