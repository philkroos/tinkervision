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

    TV_Int module_id_;  ///< Some id

    bool active_{false};  ///< True if the module is to be executed

    ModuleWrapper::Tag tags_{ModuleWrapper::Tag::None};  ///< Runtime tags used
                                                         /// by the mainloop

    Module* tv_module_;  ///< Wrapped module

    TV_Callback cb_ = nullptr;  ///< Callback for results of the wrapped module

    uint8_t period_{1};  ///< An execution frequency for the wrapped module.
                         /// Defaults to 1, which means 'execute every cycle'.
                         /// Set to zero, the module would not execute at all.

public:
    ModuleWrapper(Module* executable, TV_Int module_id,
                  std::string const& load_path)
        : load_path_(load_path),
          module_id_(module_id),
          tv_module_(executable) {}

    ~ModuleWrapper(void) { Log("MODULE::Destructor", name()); }

    // No copy allowed
    ModuleWrapper(ModuleWrapper const& other) = delete;
    ModuleWrapper(ModuleWrapper&& other) = delete;
    ModuleWrapper& operator=(ModuleWrapper const& rhs) = delete;
    ModuleWrapper& operator=(ModuleWrapper&& rhs) = delete;

    bool register_callback(TV_Callback callback) {
        if (not result() or cb_) {
            return false;
        }

        Log("MODULE", "Set callback for ", name(), " (ID ", module_id_, ")");
        cb_ = callback;
        return true;
    }

    /// Execute the wrapped module with the given image.
    /// \param[in] image The current frame
    void execute(tv::Image const& image);

    TV_Int id(void) const { return module_id_; }
    std::string name(void) const;
    ModuleType const& type(void) const;

    bool running(void) const noexcept;

    bool enabled(void) const noexcept { return active_; }

    // return false if previous state was the same
    bool enable(void) noexcept { return switch_active(true); }

    // return false if previous state was the same
    bool disable(void) noexcept { return switch_active(false); }

    // return false if previous state was the same
    bool switch_active(bool to_state) {
        Log("MODULE", to_state ? "Enabling " : "Disabling ", "module ",
            module_id_, " (was ", active_ ? "active)" : "inactive)");
        auto was_active = active_;
        active_ = to_state;
        return not(was_active == active_);
    }

    void exec(tv::Image& image) { execute(image); }

    TV_Callback callback(void) const { return cb_; }

    ColorSpace expected_format(void) const;

    /// Get the list of parameters valid for this module.
    /// \todo This should return the allowed min/max value per parameter as
    /// well.
    /// \param[inout] parameter The list of parameters.
    void get_parameters_list(std::vector<std::string>& parameter) const;

    /// Check if a module supports a parameter.
    /// \param[in] parameter Name of the parameter.
    /// \return true if the parameter is supported.
    bool has_parameter(std::string const& parameter) const;

    /// Get the current value of a parameter. It is expected that the parameter
    /// exists. The value returned if it doesn't is module specific. So a caller
    /// should have called has_parameter() in advance.
    /// \param[in] parameter The name of the parameter.
    /// \return The value of the parameter.
    TV_Word get_parameter(std::string const& parameter);

    /// Set the value of a parameter.
    /// \param[in] parameter The name of the parameter.
    /// \param[in] value The value.
    /// \return true, if the parameter has value \c value now. This might fail
    /// if the range of the parameter is limited.
    bool set_parameter(std::string const& parameter, TV_Word value);

    Result const* result(void) const;

    Tag const& tags(void) const { return tags_; }
    void tag(Tag tags) { tags_ |= tags; }

    Module* executable(void) { return tv_module_; }
};
}
#endif
