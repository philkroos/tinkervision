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

namespace tfv {

class Module {
private:
    std::string type_;
    bool active_;

protected:
    TFV_Int module_id_;
    TFV_Bool marked_for_removal_;  ///< if set, the Api will remove this module

    Module(TFV_Int module_id, std::string type)
        : type_{type},
          active_{true},
          module_id_{module_id},
          marked_for_removal_{false} {}

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

    bool is_active(void) const noexcept { return active_; }
    void activate(void) noexcept { active_ = true; }
    void deactivate(void) noexcept { active_ = false; }

    bool marked_for_removal(void) const noexcept { return marked_for_removal_; }
    void mark_for_removal(void) noexcept { marked_for_removal_ = true; }

    virtual bool is_executable(void) const noexcept = 0;
};

template <typename T>
struct dependent_false : std::false_type {};

// Interface methods to be implemented by modules

template <typename T, typename... Args>
bool valid(Args&... args) {

    // compiler message if this method is undefined for a module
    static_assert(dependent_false<T>::value, "Undefined interface valid");

    // will never occur
    return false;
}
}
#endif
