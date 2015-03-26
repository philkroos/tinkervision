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

#ifndef COLORTRACKING_H
#define COLORTRACKING_H

#ifdef DEV
#include <iostream>
#endif  // DEV
#ifdef DEBUG_COLORTRACKING
#include "window.hh"
#endif

#include "module.hh"
#include "image.hh"

namespace tfv {

struct Colortracking : public BGRModule {
    TFV_Byte min_hue;
    TFV_Byte max_hue;
    TFV_CallbackColortrack callback;
    TFV_Context context;
    TFV_Byte min_hue0;
    TFV_Byte max_hue0;
    TFV_Byte min_saturation = 50;
    TFV_Byte min_value = 50;
    TFV_Byte max_saturation = 255;
    TFV_Byte max_value = 255;

    Colortracking(TFV_Id module_id, TFV_Byte min_hue, TFV_Byte max_hue,
                  TFV_CallbackColortrack callback, TFV_Context context)
        : BGRModule(module_id),
          min_hue(min_hue),
          max_hue(max_hue),
          callback(callback),
          context(context) {
#ifdef DEV
        std::cout << "Init colortracking id " << module_id << std::endl;
#endif  // DEV
        if (min_hue > max_hue) {
            min_hue0 = 0;
            max_hue0 = COLORTRACK_MAXIMUM_HUE;
        }
    }

    virtual ~Colortracking(void){};
    Colortracking(Colortracking const& other) = delete;
    Colortracking(Colortracking&& other) = delete;
    Colortracking& operator=(Colortracking const& rhs) = delete;
    Colortracking& operator=(Colortracking&& rhs) = delete;

    virtual void execute(tfv::Image const& image);

#ifdef DEBUG_COLORTRACKING
    tfv::Window window;
#endif  // DEBUG_COLORTRACKING
};

// called with the exact same parameter list as the constructor,
// and all args are references. If this is not correct
// here, the 'basecase' in module.hh will be used.
template <>
bool valid<Colortracking>(TFV_Byte& min_hue, TFV_Byte& max_hue,
                          TFV_CallbackColortrack& callback,
                          TFV_Context& context);
template <>
void set<Colortracking>(Colortracking* ct, TFV_Byte min_hue, TFV_Byte max_hue,
                        TFV_CallbackColortrack callback, TFV_Context context);

template <>
void get<Colortracking>(Colortracking const& ct, TFV_Byte& min_hue,
                        TFV_Byte& max_hue);
};

#endif /* COLORTRACKING_H */
