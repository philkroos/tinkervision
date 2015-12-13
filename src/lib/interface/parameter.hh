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

#pragma once

#include <functional>

namespace tv {

class Module;

/// Abstract baseclass for parameters of a vision module.
/// This simplifies storage and value checking of parameters.
/// It's kind of ugly since this looks like it should be a template class.
/// However, it's best to store all parameters in a single map, so a common
/// interface is needed.
/// The baseclass therefore implements stubs for all methods that return a
/// default value and don't c hange the objects state.  E.g. a numerical value
/// can be passed to a string parameter and vice versa. This must be checked on
/// the user side (Module etc.).
/// \see Module.
class Parameter {
public:
    enum class Type : uint8_t { Numerical, String };

    virtual ~Parameter(void) = default;

    /// Return the name of this parameter.
    /// \return name_.
    std::string const& name(void) const { return name_; }

    /// Return the type of this parameter.
    /// \return name_.
    Type type(void) const { return type_; }

    virtual int32_t min(void) const { return 0; }
    virtual int32_t max(void) const { return 0; }
    virtual std::string const& string(void) const { return empty_default_; }

    virtual bool set(int32_t value) { return false; }
    virtual bool set(std::string const& value) { return false; }
    virtual bool get(int32_t& value) const { return false; }
    virtual bool get(std::string& value) const { return false; }

protected:
    /// C'tor used by Module to construct a string parameter.
    /// \param[in] type Type of this parameter.
    /// \param[in] name Name of this parameter. Keep it short.
    Parameter(Type type, std::string const& name) : type_(type), name_(name) {}

private:
    Type type_;
    std::string name_;
    std::string const empty_default_ = "";
};

/// Encapsulate a numeric parameter for use in a vision module.
/// A numerical parameter has to be constructed with the allowed range.
/// On assignment, the range will be checked. If it would be violated, no
/// assignment will be made.
class NumericalParameter : public Parameter {
public:
    ~NumericalParameter(void) override = default;

    /// Set this parameter to a new value. Check if the limits are
    /// respected, else don't set.
    /// \param[in] The new value.
    /// \return True if setting possible.
    bool set(int32_t value) override;

    /// Retrieve the current value of this parameter.
    /// \param[out] value_
    /// \return true.
    bool get(int32_t& value) const override;

    /// Return the minimum allowed value.
    /// \return min_.
    int32_t min(void) const override { return min_; }

    /// Return the maximum allowed value.
    /// \return max_.
    int32_t max(void) const override { return max_; }

private:
    friend class Module;         ///< Module can create Parameter.
    friend class ModuleWrapper;  ///< ModuleWrapper can create Parameter.
    friend class ModuleLoader;   ///< ModuleLoader can create Parameter.

    /// C'tor used by Module to construct a numerical parameter.
    /// \param[in] name Name of this parameter. Keep it short.
    /// \param[in] min Minimum value allowed for this parameter.
    /// \param[in] max Maximum value allowed for this parameter.
    /// \param[in] init Initialization value for this parameter.
    NumericalParameter(std::string const& name, int32_t min, int32_t max,
                       int32_t init);

    /// Never needed.
    Parameter& operator=(Parameter const& other) = delete;

    std::string const name_;  ///< Parameter name.
    int32_t const min_;       ///< Minimum allowed value.
    int32_t const max_;       ///< Maximum allowed value.
    int32_t value_;           ///< Current value.
};

/// Encapsulate a string parameter for use in a vision module.
class StringParameter : public Parameter {
public:
    ~StringParameter(void) override = default;

    /// Set this parameter to a new value.
    /// \param[in] The new value.
    /// \return True if setting possible.
    bool set(std::string const& value) override;

    /// Return the current value of this parameter.
    /// \param[out] value_.
    /// \return true.
    bool get(std::string& value) const override;

private:
    friend class Module;         ///< Module can create Parameter.
    friend class ModuleWrapper;  ///< ModuleWrapper can create Parameter.
    friend class ModuleLoader;   ///< ModuleLoader can create Parameter.

    /// C'tor used by Module to construct a string parameter.
    /// \param[in] name Name of this parameter. Keep it short.
    /// \param[in] init Initialization value for this parameter.
    StringParameter(std::string const& name, std::string const& init,
                    std::function<bool(std::string const& old,
                                       std::string const& value)> verify);

    std::string const name_;  ///< Parameter name.
    std::string value_;       ///< Current value.
    std::function<
        bool(std::string const& old, std::string const& value)> verify_{
        nullptr};  ///< If set, will be called before setting to verify the new
                   /// value.
};
}
