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

#include "feature.hh"

void tfv::set_feature(Feature& feature, TFV_Byte min_hue, TFV_Byte max_hue) {

    feature.min_hue = min_hue;
    feature.max_hue = max_hue;
}

void tfv::set_feature(Feature& feature, TFV_Id camera_id) {

    feature.camera_id = camera_id;
}

void tfv::set_feature(Feature& feature, TFV_Id camera_id, TFV_Byte min_hue,
                      TFV_Byte max_hue, TFV_Callback callback,
                      TFV_Context context) {
    feature.camera_id = camera_id;
    feature.min_hue = min_hue;
    feature.max_hue = max_hue;
    feature.callback = callback;
    feature.context = context;
}

bool tfv::check_configuration_settings(TFV_Byte& min_hue, TFV_Byte& max_hue,
                                       TFV_Callback callback) {
    return (min_hue < max_hue)and callback;
}

#ifdef DEV
std::string tfv::feature_as_string(Feature& f) {
    auto address = (long)&f;
    return std::string(std::to_string(address)) + " with: " + "\n-- min_hue: " +
           std::to_string(f.min_hue) + " " + "max_hue: " +
           std::to_string(f.max_hue) + " " + "tracked by camera " +
           std::to_string(f.camera_id);
}
#endif
