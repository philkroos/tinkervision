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

#include <map>
#include <string>

#include "api.hh"
#include "tinkervision.h"
#include "colortracking.hh"
#include "stream.hh"

extern "C" {

//
// General library functions
//

TFV_Result camera_available(void) {
    return tfv::get_api().is_camera_available();
}

TFV_Result stop(void) { return tfv::get_api().stop(); }

TFV_Result start(void) { return tfv::get_api().start(); }

TFV_Result quit(void) { return tfv::get_api().quit(); }

TFV_Result set_execution_latency(TFV_UInt milliseconds) {
    return tfv::get_api().set_execution_latency_ms(milliseconds);
}

TFV_String result_string(TFV_Result code) {
    return tfv::get_api().result_string(code);
}

//
// Colortracking interface
//

TFV_Result colortracking_start(TFV_Id id, TFV_Byte min_hue, TFV_Byte max_hue,
                               TFV_CallbackColortrack callback,
                               TFV_Context context) {

    return tfv::get_api().module_set<tfv::Colortracking>(id, min_hue, max_hue,
                                                         callback, context);
}

TFV_Result colortracking_restart(TFV_Id feature_id) {
    return tfv::get_api().module_start<tfv::Colortracking>(feature_id);
}

TFV_Result colortracking_stop(TFV_Id feature_id) {
    return tfv::get_api().module_stop<tfv::Colortracking>(feature_id);
}

TFV_Result colortracking_get(TFV_Id feature_id, TFV_Byte* min_hue,
                             TFV_Byte* max_hue) {
    return tfv::get_api().module_get<tfv::Colortracking>(feature_id, *min_hue,
                                                         *max_hue);
}

//
// Streamer interface
//
TFV_Result streamer_stream(TFV_Id streamer_id) {
    return tfv::get_api().module_set<tfv::Stream>(streamer_id);
}
}
