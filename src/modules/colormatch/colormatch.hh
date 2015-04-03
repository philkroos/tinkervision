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

#ifndef COLORMATCH_H
#define COLORMATCH_H

#include "executable.hh"

namespace tfv {

struct Colormatch : public BGRModule {
private:
    // arbitrary values; bringing good results
    TFV_Byte const min_saturation{50};
    TFV_Byte const min_value{50};

    // these as used in opencv, see
    // http://docs.opencv.org/modules/imgproc/doc/miscellaneous_transformations.html#cvtcolor
    TFV_Byte const min_hue{255};
    TFV_Byte const max_saturation{255};
    TFV_Byte const max_value{255};
    TFV_Byte const max_hue{180};

public:
    // configurable values
    TFV_Byte user_min_hue;  ///< The minimum value as set by the user
    TFV_Byte user_max_hue;  ///< The maximum value as set by the user
    TFV_CallbackColormatch callback;
    TFV_Context context;

    Colormatch(TFV_Int module_id, TFV_Byte min_hue, TFV_Byte max_hue,
               TFV_CallbackColormatch callback, TFV_Context context)
        : BGRModule(module_id, "Colormatch"),
          user_min_hue(min_hue),
          user_max_hue(max_hue),
          callback(callback),
          context(context) {}

    virtual ~Colormatch(void) = default;
    virtual void execute(tfv::Image const& image);
};

// called with the exact same parameter list as the constructor,
// and all args are references. If this is not correct
// here, the 'basecase' in module.hh will be used.
template <>
bool valid<Colormatch>(TFV_Byte& min_hue, TFV_Byte& max_hue,
                       TFV_CallbackColormatch& callback, TFV_Context& context);
template <>
void set<Colormatch>(Colormatch* ct, TFV_Byte min_hue, TFV_Byte max_hue,
                     TFV_CallbackColormatch callback, TFV_Context context);

template <>
void get<Colormatch>(Colormatch const& ct, TFV_Byte& min_hue,
                     TFV_Byte& max_hue);
};

#endif /* COLORMATCH_H */
