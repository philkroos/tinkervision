/// \file parameter.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Definition of the class \c Parameter.
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

#include "parameter.hh"

bool tv::NumericalParameter::set(int32_t value) {
    if (value < min_ or value > max_) {
        return false;
    }
    value_ = value;
    return true;
}

bool tv::NumericalParameter::get(int32_t& value) const {
    value = value_;
    return true;
}

bool tv::StringParameter::set(std::string const& value) {
    if (verify_ and verify_(value_, value)) {
        value_ = value;
        return true;
    }
    return false;
}

bool tv::StringParameter::get(std::string& value) const {
    value = value_;
    return true;
}

tv::NumericalParameter::NumericalParameter(std::string const& name, int32_t min,
                                           int32_t max, int32_t init)
    : Parameter(Parameter::Type::Numerical, name),
      min_(min > max ? max : min),
      max_(max),
      value_(init) {

    if (value_ > max_) {
        value_ = max_;
    } else if (value_ < min_) {
        value_ = min_;
    }
}

tv::StringParameter::StringParameter(
    std::string const& name, std::string const& init,
    std::function<bool(std::string const& old, std::string const& value)>
        verify)
    : Parameter(Parameter::Type::String, name), value_(init), verify_(verify) {}
