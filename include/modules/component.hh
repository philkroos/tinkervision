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

#ifndef COMPONENT_H
#define COMPONENT_H

#include <iostream>

#include "tinkervision_defines.h"
#include "image.hh"

namespace tfv {

struct Component {
    TFV_Id component_id;
    bool active;

    explicit Component(TFV_Id component_id)
        : component_id(component_id), active(true) {}
    virtual ~Component(void){};

    Component(Component const& other) = delete;
    Component(Component&& other) = delete;
    Component& operator=(Component const& rhs) = delete;
    Component& operator=(Component&& rhs) = delete;

    virtual ImageFormat expected_format(void) const = 0;

    // Internal part of the interface that a concrete component has to
    // implement.  The rest is defined as free function after this class
    // declaration.
    virtual void execute(tfv::Image const& image) = 0;
};

// Interface methods to be implemented by components

template <typename T, typename... Args>
bool valid(Args&... args) {
    std::cout << "Warning, valid undefined" << std::endl;
    return false;
}

template <typename T, typename... Args>
void set(T* component, Args... args) {
    std::cout << "Warning, set undefined" << std::endl;
}

template <typename T, typename... Args>
void get(T const& component, Args&... args) {
    std::cout << "Warning, get undefined" << std::endl;
}

// Specializations for different ImageFormat values. Just derive e.g. from
// BGRComponent.

template <ImageFormat Format>
struct ComponentWithImageFormat : public Component {
    ComponentWithImageFormat(TFV_Id id) : Component(id) {}
    virtual ~ComponentWithImageFormat(void) = default;
    virtual ImageFormat expected_format(void) const { return Format; }
};

struct BGRComponent : public ComponentWithImageFormat<ImageFormat::BGR888> {
    virtual ~BGRComponent(void) = default;
    BGRComponent(TFV_Id id) : ComponentWithImageFormat(id) {}
};

struct RGBComponent : public ComponentWithImageFormat<ImageFormat::RGB888> {
    virtual ~RGBComponent(void) = default;
    RGBComponent(TFV_Id id) : ComponentWithImageFormat(id) {}
};

struct YUYVComponent : public ComponentWithImageFormat<ImageFormat::YUYV> {
    virtual ~YUYVComponent(void) = default;
    YUYVComponent(TFV_Id id) : ComponentWithImageFormat(id) {}
};

struct YV12Component : public ComponentWithImageFormat<ImageFormat::YV12> {
    virtual ~YV12Component(void) = default;
    YV12Component(TFV_Id id) : ComponentWithImageFormat(id) {}
};
};
#endif /* COMPONENT_H */
