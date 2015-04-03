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
#include "colormatch.hh"
#include "motiondetect.hh"
#include "stream.hh"
#include "record.hh"

extern "C" {

//
// General library functions
//

TFV_Result camera_available(void) {
    return tfv::get_api().is_camera_available();
}

TFV_Result start_idle(void) { return tfv::get_api().start_idle(); }

TFV_Result get_resolution(TFV_Size& width, TFV_Size& height) {
    return tfv::get_api().resolution(width, height);
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
// Colormatch interface
//

TFV_Result colormatch_start(TFV_Id id, TFV_Byte min_hue, TFV_Byte max_hue,
                            TFV_CallbackColormatch callback,
                            TFV_Context context) {

    return tfv::get_api().module_set<tfv::Colormatch>(id, min_hue, max_hue,
                                                      callback, context);
}

TFV_Result colormatch_restart(TFV_Id feature_id) {
    return tfv::get_api().module_start<tfv::Colormatch>(feature_id);
}

TFV_Result colormatch_stop(TFV_Id feature_id) {
    return tfv::get_api().module_stop<tfv::Colormatch>(feature_id);
}

TFV_Result colormatch_get(TFV_Id feature_id, TFV_Byte* min_hue,
                          TFV_Byte* max_hue) {
    return tfv::get_api().module_get<tfv::Colormatch>(feature_id, *min_hue,
                                                      *max_hue);
}

//
// Motiondetect interface
//

TFV_Result motiondetect_start(TFV_Id id, TFV_CallbackMotiondetect callback,
                              TFV_Context context) {

    return tfv::get_api().module_set<tfv::Motiondetect>(id, callback, context);
}

//
// Streamer interface
//

TFV_Result streamer_stream(TFV_Id streamer_id) {
    return tfv::get_api().module_set<tfv::Stream>(streamer_id);
}

//
// Record interface
//

TFV_Result snapshot(void) { return tfv::get_api().module_set<tfv::Snapshot>(); }
}
