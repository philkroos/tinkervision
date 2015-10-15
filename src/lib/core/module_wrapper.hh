/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014-2015 philipp.kroos@fh-bielefeld.de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

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
    bool active_;
    ModuleWrapper::Tag tags_ = ModuleWrapper::Tag::None;
    TV_Int module_id_;

    Module* tv_module_;

    TV_Callback cb_ = nullptr;

public:
    ModuleWrapper(Module* executable, TV_Int module_id)
        : active_(false), module_id_(module_id), tv_module_(executable) {}

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

    void get_parameters_list(std::vector<std::string>& parameter) const;

    bool has_parameter(std::string const& parameter) const;

    bool set(std::string const& parameter, TV_Word value);

    TV_Word get(std::string const& parameter);

    bool set_parameter(std::string const& parameter, TV_Word value) {
        return set(parameter, value);
    }

    void get_parameter(std::string const& parameter, TV_Word& value) {
        value = get(parameter);
    }

    Result const* result(void) const;

    Tag const& tags(void) const { return tags_; }
    void tag(Tag tags) { tags_ |= tags; }

    Module* executable(void) { return tv_module_; }
};
}
#endif
