/// \file pixel.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of \c Pixel, part of the gesture module.
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

#include <cstdint>
#include <cstddef>

struct Pixel {
    uint16_t x, y;
    size_t idx;
    Pixel(uint16_t x, uint16_t y, size_t idx) : x(x), y(y), idx(idx) {}
};
