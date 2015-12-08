/// \file find_object.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of \c FindObject, part of the gesture module.
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

#ifndef FIND_OBJECT_H
#define FIND_OBJECT_H

#include <vector>

#include "pixel.hh"
#include <tinkervision/image.hh>

/// Find the largest connected object.
/// Based on the method described in Watershed:
/// http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=87344, see
/// https://en.wikipedia.org/wiki/Connected-component_labeling
class FindObject {
private:
    uint16_t width_;
    uint16_t height_;
    uint16_t* labels_{nullptr};

    // A possible match
    struct Object {
        uint16_t x, y;
        uint16_t count;
        uint16_t label;
        Object(void) = default;
        Object(uint16_t x, uint16_t y, uint16_t count, uint16_t label)
            : x(x), y(y), count(count), label(label) {}
    };
    std::vector<Object> objects_;

public:
    FindObject(uint16_t width, uint16_t height);
    ~FindObject(void);

    using Result = std::vector<Pixel>;
    bool operator()(Result&, uint16_t minsize, tv::ImageData const* image);
};

#endif
