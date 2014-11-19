/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014 philipp.kroos@fh-bielefeld.de

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

#include "tinkervision_defines.h"

namespace tinkervision {

class Component {
private:
    TFV_Id camera_id_;
    bool active_;

public:
    Component(TFV_Id camera_id) : camera_id_(camera_id), active_(false) {}
    virtual ~Component(void) = default;

    Component(Component const& other) = default;
    Component(Component&& other) = default;
    Component& operator=(Component const& rhs) = default;
    Component& operator=(Component&& rhs) = default;

    bool active(void) const { return active_; }
    TFV_Id camera_id(void) const { return camera_id_; }

    void activate(void) { active_ = true; }
    void deactivate(void) { active_ = false; }

    virtual void execute(TFV_ImageData* data, TFV_Int rows,
                         TFV_Int columns) = 0;
};

template <typename T, typename... Args>
bool valid(Args&... args) {
    return false;
}
};
#endif /* COMPONENT_H */
