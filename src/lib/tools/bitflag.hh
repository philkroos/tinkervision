/// \file parameter.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Helpermethods to perform bitwise operations on a c++11-style
/// scoped-enum.
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

#ifndef BITFLAG_H
#define BITFLAG_H

#include <type_traits>

template <typename Bits>
inline constexpr Bits operator|(Bits const& lhs, Bits const& rhs) {
    using T = typename std::underlying_type<Bits>::type;
    return static_cast<Bits>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

template <typename Bits>
inline bool operator&(Bits const& lhs, Bits const& rhs) {
    using T = typename std::underlying_type<Bits>::type;
    return static_cast<T>(lhs) & static_cast<T>(rhs);
}

template <typename Bits>
inline Bits& operator|=(Bits& lhs, Bits const& rhs) {
    using T = typename std::underlying_type<Bits>::type;
    lhs = static_cast<Bits>(static_cast<T>(lhs) | static_cast<T>(rhs));
    return lhs;
}

#endif
