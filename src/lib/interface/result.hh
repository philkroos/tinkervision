/// \file result.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of \c Result.
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

namespace tv {

/// Result is the unified possible return value of vision modules.
/// In general, it could represent a value (if only x is set), a point (if x and
/// y are set), a rectangle (width and height also set) or a string.  A
/// numerical value is 'unset' if it equals -1.  The string is unset if it is
/// empty.  If neither x nor the string are set, the result is considered
/// invalid.  The result is also invalid if the maximum string length of
/// (TV_STRING_SIZE - 1) is exceeded.  Other assumptions are not made, so the
/// considerations above can be ignored, e.g. to return a string and a value or
/// three values which do not represent a location.  This has to be documented
/// with the module producing such a result.  The result will always be returned
/// completely, regardless of any unset values.
struct Result {
    int32_t x{-1};           /// First value, -1 means unset.
    int32_t y{-1};           /// Second value, -1 means unset.
    int32_t width{-1};       /// Third value, -1 means unset.
    int32_t height{-1};      /// Fourth value, -1 means unset.
    std::string result{""};  /// Fifth value, "" means unset.

    /// A result is valid if at least x is > 0 or the string has been set.
    operator bool(void) const {
        return x > 0 or (result != "" and result.size() < TV_STRING_SIZE);
    }
};
}
