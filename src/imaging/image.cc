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

#include <algorithm>  // copy

#include "logger.hh"

tfv::Image& tfv::Image::operator=(tfv::Image const& other) {
    width = other.width;
    height = other.height;
    bytesize = other.bytesize;
    timestamp = other.timestamp;
    format = other.format;
    if (bytesize) {
        data_ = other.data_;
        foreign_data_ = other.foreign_data_;
    }

    if (not foreign_data_) {
        LogWarning("IMAGE", "Created a flat copy of allocated data");
    }

    return *this;
}

tfv::Image::~Image(void) {
    if (data_ and not foreign_data_) {
        delete[] data_;
    } else {
        LogWarning("IMAGE", "Destructing without deleting data");
    }
}

void tfv::Image::copy_to(tfv::Image& other) const {
    if (other.bytesize != bytesize or other.width != width) {
        if (other.data_) {
            other.delete_data();
        }
        other = *this;
    } else {
        if (not other.data_) {
            other.data_ = new ImageData[bytesize];
            other.foreign_data_ = false;
        }
        std::copy_n(data_, bytesize, other.data_);
    }
}

void tfv::Image::set(tfv::ImageData* data, size_t bytesize) {
    if (this->data_ and bytesize != this->bytesize) {
        delete_data();
    }
    this->bytesize = bytesize;
    this->data_ = data_;
    foreign_data_ = true;
}

void tfv::Image::init(size_t width, size_t height, size_t bytesize) {
    if (data_) {
        delete_data();
    }
    this->width = width;
    this->height = height;
    this->bytesize = bytesize;
    this->data_ = new ImageData[bytesize];
    foreign_data_ = false;
}

void tfv::Image::copy(tfv::ImageData* data, size_t width, size_t height,
                      size_t bytesize) {
    if (this->data_ and bytesize != this->bytesize) {
        delete_data();
        this->bytesize = bytesize;
        this->data_ = new ImageData[bytesize];
    }
    this->width = width;
    this->height = height;
    std::copy_n(data_, bytesize, this->data_);
    foreign_data_ = false;
}

tfv::Image::Image(tfv::Image const& other) {
    width = other.width;
    height = other.height;
    bytesize = other.bytesize;
    timestamp = other.timestamp;
    format = other.format;
    if (bytesize) {
        data_ = new ImageData(bytesize);
        std::copy_n(data_, bytesize, data_);
        foreign_data_ = false;
    }
}

tfv::Image::Image(tfv::Image&& other) {
    width = other.width;
    height = other.height;
    bytesize = other.bytesize;
    timestamp = other.timestamp;
    format = other.format;
    data_ = other.data_;
    other.data_ = nullptr;
    foreign_data_ = other.foreign_data_;
}

tfv::Image& tfv::Image::operator=(tfv::Image&& other) {
    width = other.width;
    height = other.height;
    bytesize = other.bytesize;
    timestamp = other.timestamp;
    format = other.format;
    data_ = other.data_;
    other.data_ = nullptr;
    foreign_data_ = other.foreign_data_;
    return *this;
}

void tfv::Image::delete_data(void) {
    if (data_ and not foreign_data_) {
        delete[] data_;
        data_ = nullptr;
    }
}
