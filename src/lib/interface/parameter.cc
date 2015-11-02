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

bool tv::Parameter::set(parameter_t value) {
    if (value < min_ or value > max_) {
        return false;
    }
    value_ = value;
    return true;
}

tv::Parameter::Parameter(std::string const& name, parameter_t min,
                         parameter_t max, parameter_t init)
    : name_(name), min_(min), max_(max), value_(init) {

    if (min_ > max_) {
        LogError("PARAMETER", "Min > Max ", min_, " ", max_);
    }
    if (value_ > max_ or value_ < min_) {
        LogError("PARAMETER", "Init: ", value_);
    }
}
