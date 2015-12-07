/// \file module_wrapper.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of the class \c ModuleWrapper.
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

#ifndef MODULE_WRAPPER_H
#define MODULE_WRAPPER_H

#include <typeinfo>
#include <type_traits>
#include <cassert>
#include <vector>

#include "tinkervision_defines.h"
#include "image.hh"
#include "bitflag.hh"
#include "logger.hh"
#include "module.hh"

namespace tv {

struct Result;
enum class ModuleType : uint8_t;

class ModuleWrapper {
public:
    using Constructor = Module* (*)(Environment const&);
    using Destructor = void (*)(Module*);

    // runtime tags describing some sort of status this module is in, as
    // relevant for the execution of the Api
    enum class Tag : unsigned {
        None = 0x01,
        ExecAndRemove = 0x02,
        ExecAndDisable = 0x04,
        Removable = 0x08,
        Sequential = 0x10,
    };

private:
    std::string const load_path_;  ///< Path the wrapped module was loaded from

    int16_t module_id_;  ///< Some id

    bool initialized_{
        false};  ///< True if the module can be executed, set during initialize.

    bool active_{
        false};  ///< True if the module is running (i.e. will be executed)

    ModuleWrapper::Tag tags_{ModuleWrapper::Tag::None};  ///< Runtime tags used
                                                         /// by the mainloop

    Module* tv_module_{nullptr};  ///< Wrapped module
    Result latest_result_;        ///< Set after execute if provided

    TV_Callback cb_ = nullptr;  ///< Callback for results of the wrapped module
    bool callbacks_enabled_{true};  ///< If false, callbacks won't be made. This
                                    /// has only relevance if the wrapped module
                                    /// can_have_result()

    uint8_t period_{1};  ///< An execution frequency for the wrapped module.
                         /// Defaults to 1, which means 'execute every cycle'.
                         /// Set to zero, the module would not execute at all.

    Destructor dtor_;

public:
    ModuleWrapper(Constructor ctor, Destructor dtor, int16_t module_id,
                  Environment const& envir, std::string const& load_path)
        : load_path_(load_path),
          module_id_(module_id),
          tv_module_(ctor(envir)),
          dtor_(dtor) {}

    ~ModuleWrapper(void) {
        Log("MODULE::Destructor", name());
        dtor_(tv_module_);
    }

    // No copy allowed
    ModuleWrapper(ModuleWrapper const& other) = delete;
    ModuleWrapper(ModuleWrapper&& other) = delete;
    ModuleWrapper& operator=(ModuleWrapper const& rhs) = delete;
    ModuleWrapper& operator=(ModuleWrapper&& rhs) = delete;

    bool register_callback(TV_Callback callback) {
        if (not tv_module_->can_have_result()) {
            return false;
        }

        Log("MODULE", "Set callback for ", name(), " (ID ", module_id_, ")");
        cb_ = callback;
        return true;
    }

    /// Execute the wrapped module with the given image.
    /// \param[in] image The current frame
    void execute(tv::Image const& image);

    /// Try to initialize the wrapped module.
    /// \return \c false if initialization failed.
    bool initialize(void) {

        /// This can only be run once.
        if (initialized_) {
            return false;
        }

        initialized_ = true;
        if (tv_module_->produces_result()) {
            initialized_ =
                tv_module_->register_parameter("result_timeout", 0, 40, 20) and
                tv_module_->register_parameter("callbacks_enabled", 0, 1, 1);
        }

        initialized_ = initialized_ and
                       tv_module_->register_parameter("period", 0, 500, 1) and
                       tv_module_->initialize();

        return initialized_;
    }

    int16_t id(void) const { return module_id_; }
    std::string name(void) const;
    ModuleType const& type(void) const;

    bool enabled(void) const { return active_; }

    /// Enable the wrapped module.
    /// \return True if the module could be activated.
    bool enable(void) {
        Log("MODULE", "Enabling ", module_id_, " (", name(), ")");
        if (initialized_) {
            active_ = true;
        } else {
            active_ = false;
        }
        return active_;
    }

    /// Enable the wrapped module.
    /// If the module is not currently enabled, also set Tag::ExecAndDisable.
    /// \return True if the module could be activated.
    bool enable_at_least_once(void) {
        Log("MODULE", "Enabling ", module_id_, " (", name(), ")");
        if (initialized_) {
            if (not active_) {
                tag(Tag::ExecAndDisable);
            }
            active_ = true;
        } else {
            active_ = false;
        }
        return active_;
    }

    /// Disable this unit. This does not modify the wrapped module, simply stops
    /// it from being executed.
    void disable(void) {
        Log("MODULE", "Disabling ", module_id_, " (", name(), ")");
        tv_module_->stop();
        active_ = false;
    }

    TV_Callback callback(void) const { return cb_; }

    ColorSpace expected_format(void) const;

    /// Get the list of parameters valid for this module.
    /// \return The list of parameters.
    void get_parameters_list(std::vector<Parameter const*>& parameters) const;

    /// Check if a module supports a parameter.
    /// \param[in] parameter Name of the parameter.
    /// \return true if the parameter is supported.
    bool has_parameter(std::string const& parameter) const;

    /// Get the current value of a parameter.
    /// \param[in] parameter The name of the parameter.
    /// \param[out] value Will be set accordingly on success.
    /// \return True if such a parameter exists (value is valid and type is T).
    template <typename T>
    bool get_parameter(std::string const& parameter, T& value) {
        return tv_module_->get(parameter, value);
    }

    /// Set the value of a parameter.
    /// \param[in] parameter The name of the parameter.
    /// \param[in] value The value.
    /// \return true, if the parameter has value \c value now. This might fail
    /// if the range of the parameter is limited.
    bool set_parameter(std::string const& parameter, int32_t value);

    /// Set the value of a string parameter.
    /// \param[in] parameter The name of the parameter.
    /// \param[in] value The value.
    /// \return true, if the parameter has value \c value now. This might fail
    /// if the range of the parameter is limited.
    bool set_parameter(std::string const& parameter, std::string const& value);

    Parameter const& get_parameter_by_number(size_t number) const {
        return tv_module_->get_parameter_by_number(number);
    }

    /// Get the number of parameters the wrapped module provides.
    /// \return Count of available parameters.
    size_t get_parameter_count(void) const {
        return tv_module_->parameter_count();
    }

    /// Get a specific parameter from the wrapped module.
    /// \param[in] number A value < get_parameter_count().
    /// \return The parameter that has been assigned the number specified
    /// internally. If number exceeds get_parameter_count(), the one with the
    /// largest number.
    Parameter const& describe_parameter_by_number(size_t number) const {
        return tv_module_->get_parameter_by_number(number);
    }

    /// Retrieve the result of the latest execute().
    /// \return Latest or an invalid result.
    Result const& result(void) const;

    tv::Image const& modified_image(void) {
        return tv_module_->modified_image();
    }

    Tag const& tags(void) const { return tags_; }
    void tag(Tag tags) { tags_ |= tags; }

    Module* executable(void) { return tv_module_; }
};
}
#endif
