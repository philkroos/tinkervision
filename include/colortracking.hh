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

#ifndef COLORTRACKING_H
#define COLORTRACKING_H

#include "component.hh"

namespace tinkervision {
struct Colortracking : public Component {
    TFV_Byte min_hue;
    TFV_Byte max_hue;
    TFV_Callback callback;
    TFV_Context context;

    Colortracking(TFV_Id camera_id, TFV_Byte min_hue, TFV_Byte max_hue,
                  TFV_Callback callback, TFV_Context context)
        : Component(camera_id),
          min_hue(min_hue),
          max_hue(max_hue),
          callback(callback),
          context(context) {}

    virtual ~Colortracking(void) = default;
    Colortracking(Colortracking const& other) = delete;
    Colortracking(Colortracking&& other) = delete;
    Colortracking& operator=(Colortracking const& rhs) = delete;
    Colortracking& operator=(Colortracking&& rhs) = delete;

    virtual void execute(TFV_ImageData* data, TFV_Int rows, TFV_Int columns);
};

// called with the exact same parameter list as the constructor minus
// the camera_id, and all args are references. If this is not correct
// here, the 'basecase' in component.hh will be used.
template <>
bool valid<Colortracking>(TFV_Byte& min_hue, TFV_Byte& max_hue,
                          TFV_Callback& callback, TFV_Context& context);
template <>
void set<Colortracking>(Colortracking* ct, TFV_Byte min_hue, TFV_Byte max_hue,
                        TFV_Callback callback, TFV_Context context);
};

#endif /* COLORTRACKING_H */
