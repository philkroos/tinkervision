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
    TFV_Id module_id_;

    Module(TFV_Id module_id, std::string type)
        : type_(type), active_(true), module_id_(module_id) {}

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

    virtual ColorSpace expected_format(void) const = 0;

    // Internal part of the interface that a concrete module has to
    // implement.  The rest is defined as free function after this class
    // declaration.
    virtual void execute(tfv::Image const& image) = 0;

    bool active(void) const { return active_; }
    void activate(void) { active_ = true; }
    void deactivate(void) { active_ = false; }
};

// Interface methods to be implemented by modules

template <typename T, typename... Args>
bool valid(Args&... args) {
    std::cout << "Warning, valid undefined" << std::endl;
    return false;
}

template <typename T, typename... Args>
void set(T* module, Args... args) {
    std::cout << "Warning, set undefined" << std::endl;
}

template <typename T, typename... Args>
void get(T const& module, Args&... args) {
    std::cout << "Warning, get undefined" << std::endl;
}

// Specializations for different ColorSpace values. Just derive e.g. from
// BGRModule.

template <ColorSpace Format>
struct ModuleWithColorSpace : public Module {
    ModuleWithColorSpace(TFV_Id id, std::string type) : Module(id, type) {}
    virtual ~ModuleWithColorSpace(void) = default;
    virtual ColorSpace expected_format(void) const { return Format; }
};

struct BGRModule : public ModuleWithColorSpace<ColorSpace::BGR888> {
    virtual ~BGRModule(void) = default;
    BGRModule(TFV_Id id, std::string type) : ModuleWithColorSpace(id, type) {}
};

struct RGBModule : public ModuleWithColorSpace<ColorSpace::RGB888> {
    virtual ~RGBModule(void) = default;
    RGBModule(TFV_Id id, std::string type) : ModuleWithColorSpace(id, type) {}
};

struct YUYVModule : public ModuleWithColorSpace<ColorSpace::YUYV> {
    virtual ~YUYVModule(void) = default;
    YUYVModule(TFV_Id id, std::string type) : ModuleWithColorSpace(id, type) {}
};

struct YV12Module : public ModuleWithColorSpace<ColorSpace::YV12> {
    virtual ~YV12Module(void) = default;
    YV12Module(TFV_Id id, std::string type) : ModuleWithColorSpace(id, type) {}
};
};
#endif
