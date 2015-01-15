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

#include <map>
#include <string>

#include "api.hh"
#include "tinkervision.h"
#include "colortracking.hh"

extern "C" {

TFV_Result colortracking_start(TFV_Id id, TFV_Id camera_id, TFV_Byte min_hue,
                               TFV_Byte max_hue,
                               TFV_CallbackColortrack callback,
                               TFV_Context context) {

    try {
        auto& api = tfv::get_api();
        return api.component_set<tfv::Colortracking>(
            id, camera_id, min_hue, max_hue, callback, context);
    }
    catch (...) {
        return TFV_INTERNAL_ERROR;
    }
}

TFV_Result colortracking_restart(TFV_Id feature_id) {
    try {
        auto& api = tfv::get_api();
        return api.component_start<tfv::Colortracking>(feature_id);
    }
    catch (...) {
        return TFV_INTERNAL_ERROR;
    }
}

TFV_Result colortracking_stop(TFV_Id feature_id) {
    try {
        auto& api = tfv::get_api();
        return api.component_stop<tfv::Colortracking>(feature_id);
    }
    catch (...) {
        return TFV_INTERNAL_ERROR;
    }
}

TFV_Result colortracking_get(TFV_Id feature_id, TFV_Id* camera_id,
                             TFV_Byte* min_hue, TFV_Byte* max_hue) {
    try {
        auto& api = tfv::get_api();
        return api.component_get<tfv::Colortracking>(feature_id, *camera_id,
                                                     *min_hue, *max_hue);
    }
    catch (...) {
        return TFV_INTERNAL_ERROR;
    }
}

TFV_Result camera_available(TFV_Id camera_id) {
    try {
        auto& api = tfv::get_api();
        return api.is_camera_available(camera_id);
    }
    catch (...) {
        return TFV_INTERNAL_ERROR;
    }
}

TFV_Result stop_api(void) {
    auto& api = tfv::get_api();
    api.stop();
    return TFV_OK;
}

TFV_Result set_execution_latency(TFV_UInt milliseconds) {
    auto& api = tfv::get_api();
    return api.set_execution_latency_ms(milliseconds);
}

TFV_String result_string(TFV_Result code) {
    auto& api = tfv::get_api();
    return api.result_string(code);
}
}
