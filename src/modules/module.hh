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

#ifndef MODULE_H
#define MODULE_H

#include <iostream>  // for development

#include "tinkervision_defines.h"
#include "image.hh"
#include "bitflag.hh"

namespace tfv {

class Module {
public:
    enum class Tag : unsigned {
        None = 0x00,
        Executable = 0x01,
        Fx = 0x02,
        ExecAndRemove = 0x04,
        ExecAndDisable = 0x08,
        Removable = 0x10
    };

private:
    std::string type_;
    bool active_;
    Module::Tag tags_;

protected:
    TFV_Int module_id_;

    Module(TFV_Int module_id, std::string type, Tag tags)
        : type_{type}, active_{true}, tags_{tags}, module_id_{module_id} {}

    Module(TFV_Int module_id, std::string type)
        : Module(module_id, type, Tag::None) {}

public:
    virtual ~Module(void) {
        std::cout << "Destroying module " << module_id_ << " (" << type_ << ")"
                  << std::endl;
    }

    // No copy allowed
    Module(Module const& other) = delete;
    Module(Module&& other) = delete;
    Module& operator=(Module const& rhs) = delete;
    Module& operator=(Module&& rhs) = delete;

    bool enabled(void) const noexcept { return active_; }
    void enable(void) noexcept { active_ = true; }
    void disable(void) noexcept { active_ = false; }

    Tag const& tags(void) const { return tags_; }
    void tag(Tag tags) { tags_ |= tags; }
};

// helper for compile time check
template <typename T>
struct false_for_type : std::false_type {};

// Interface methods to be implemented by modules

template <typename T, typename... Args>
bool valid(Args&... args) {

    // compiler message if this method is undefined for a module
    static_assert(false_for_type<T>::value, "Undefined interface valid");

    // will never occur
    return false;
}
}
#endif
