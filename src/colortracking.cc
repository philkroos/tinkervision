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

#include "colortracking.hh"

#ifdef DEV
#include <iostream>
#endif

void tfv::Colortracking::execute(TFV_ImageData* data, TFV_Int rows,
                                 TFV_Int columns) {
    callback(component_id, nullptr, nullptr, nullptr, nullptr, 0, nullptr);
}

template <>
bool tfv::valid<tfv::Colortracking>(TFV_Byte& min_hue, TFV_Byte& max_hue,
                                    TFV_Callback& callback,
                                    TFV_Context& context) {
    return min_hue < max_hue and callback;
}

template <>
void tfv::set<tfv::Colortracking>(tfv::Colortracking* ct, TFV_Byte min_hue,
                                  TFV_Byte max_hue, TFV_Callback callback,
                                  TFV_Context context) {

    ct->min_hue = min_hue;
    ct->max_hue = max_hue;
    ct->callback = callback;
    ct->context = context;
}
