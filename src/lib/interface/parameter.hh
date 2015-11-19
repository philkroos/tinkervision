/// \file parameter.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of the class \c Parameter.
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

#include "logger.hh"
#include "tinkervision_defines.h"

namespace tv {

class Module;

/// Encapsulates a parameter for use in a vision module.
/// This simplifies storage and value checking of parameters.
/// A parameter has to be constructed with the allowed range.
/// On assignment, the range will be checked. If it would be violated, no
/// assignment will be made.
class Parameter {
public:
    /// Set this parameter to a new value. Check if the limits are respected,
    /// else don't set.
    /// \param[in] The new value.
    /// \return True if setting possible.
    bool set(int32_t value);

    /// Return the current value of this parameter.
    /// \return value_.
    int32_t get(void) const { return value_; }

    /// Return the name of this parameter.
    /// \return name_.
    std::string const& name(void) const { return name_; }

    /// Return the minimum allowed value.
    /// \return min_.
    int32_t min(void) const { return min_; }

    /// Return the maximum allowed value.
    /// \return max_.
    int32_t max(void) const { return max_; }

private:
    friend class Module;         ///< Module can create Parameter.
    friend class ModuleWrapper;  ///< ModuleWrapper can create Parameter.

    /// C'tor used by Module to construct a parameter.
    /// \param[in] name Name of this parameter. Keep it short.
    /// \todo Check somewhere that length possible in the redbrick-api is
    /// respected here.
    /// \param[in] min Minimum value allowed for this parameter.
    /// \param[in] max Maximum value allowed for this parameter.
    /// \param[in] init Initialization value for this parameter.
    Parameter(std::string const& name, int32_t min, int32_t max, int32_t init);

    /// Never needed.
    Parameter& operator=(Parameter const& other) = delete;

    std::string const name_;  ///< Parameter name.
    int32_t const min_;       ///< Minimum allowed value.
    int32_t const max_;       ///< Maximum allowed value.
    int32_t value_;           ///< Current value.
};
}
