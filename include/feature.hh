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

#ifndef FEATURE_H
#define FEATURE_H

#include "tinkervision_defines.h"

namespace tfv {

typedef struct Feature_ {
    TFV_Id camera_id;
    TFV_Byte min_hue;
    TFV_Byte max_hue;
    TFV_Callback callback;
    TFV_Context context;

    Feature_(TFV_Id camera_id, TFV_Byte min_hue, TFV_Byte max_hue,
             TFV_Callback callback, TFV_Context context)
        : camera_id(camera_id),
          min_hue(min_hue),
          max_hue(max_hue),
          callback(callback),
          context(context) {}
} Feature;

void set_feature(Feature& feature, TFV_Byte min_hue, TFV_Byte max_hue);

void set_feature(Feature& feature, TFV_Id camera_id);

void set_feature(Feature& feature, TFV_Id camera_id, TFV_Byte min_hue,
                 TFV_Byte max_hue, TFV_Callback callback, TFV_Context context);

bool check_configuration_settings(TFV_Byte& min_hue, TFV_Byte& max_hue,
                                  TFV_Callback callback);
#ifdef DEV
std::string feature_as_string(Feature& f);
#endif
};

#endif /* FEATURE_H */
