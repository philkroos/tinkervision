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
#include "stream.hh"

extern "C" {

//
// General library functions
//

TFV_Result camera_available(void) {
    return tfv::get_api().is_camera_available();
}

TFV_Result preselect_framesize(TFV_Size width, TFV_Size height) {
    return tfv::get_api().preselect_framesize(width, height);
}

TFV_Result start_idle(void) { return tfv::get_api().start_idle(); }

TFV_Result get_resolution(TFV_Size* width, TFV_Size* height) {
    return tfv::get_api().resolution(*width, *height);
}

TFV_Result stop_module(TFV_Id id) { return tfv::get_api().stop_id(id); }

TFV_Result stop(void) { return tfv::get_api().stop(); }

TFV_Result start(void) { return tfv::get_api().start(); }

TFV_Result scene_from_module(TFV_Id module, TFV_Scene* scene_id) {
    return tfv::get_api().scene_start(module, scene_id);
}

TFV_Result scene_add_module(TFV_Scene scene, TFV_Id module) {
    return tfv::get_api().add_to_scene(scene, module);
}

TFV_Result scene_remove(TFV_Scene scene) {
    return tfv::get_api().scene_remove(scene);
}

TFV_Result quit(void) { return tfv::get_api().quit(); }

TFV_Result set_execution_latency(TFV_UInt milliseconds) {
    return tfv::get_api().set_execution_latency_ms(milliseconds);
}

TFV_String result_string(TFV_Result code) {
    return tfv::get_api().result_string(code);
}

TFV_Result set_parameter(TFV_Id module_id, TFV_String const parameter,
                         TFV_Int value) {
    return tfv::get_api().set_parameter(module_id, parameter, value);
}

TFV_Result get_parameter(TFV_Id module_id, TFV_String const parameter,
                         TFV_Int* value) {
    return tfv::get_api().get_parameter(module_id, parameter, value);
}

TFV_Result restart_id(TFV_Id id) { return tfv::get_api().start_id(id); }

TFV_Result stop_id(TFV_Id id) { return tfv::get_api().stop_id(id); }

TFV_Result module_start(TFV_String name, TFV_Id id) {

    return tfv::get_api().module_load(name, id);
}

TFV_Result module_stop(TFV_Id id) { return tfv::get_api().module_destroy(id); }

//
// Streamer interface
//

TFV_Result streamer_stream(TFV_Id streamer_id) {
    return tfv::get_api().module_set<tfv::Stream>(streamer_id);
}

//
// Callbacks
//

TFV_Result set_value_callback(TFV_Id module, TFV_CallbackValue callback) {
    return tfv::get_api().callback_set(module, callback);
}
TFV_Result set_point_callback(TFV_Id module, TFV_CallbackPoint callback) {
    return tfv::get_api().callback_set(module, callback);
}
TFV_Result set_rect_callback(TFV_Id module, TFV_CallbackRectangle callback) {
    return tfv::get_api().callback_set(module, callback);
}
TFV_Result set_string_callback(TFV_Id module, TFV_CallbackString callback) {
    return tfv::get_api().callback_set(module, callback);
}

TFV_Result get_value_result(TFV_Id module, TFV_Size* value) {
    return TFV_NOT_IMPLEMENTED;
}
TFV_Result get_point_result(TFV_Id module, TFV_Size* x, TFV_Size* y) {
    return TFV_NOT_IMPLEMENTED;
}
TFV_Result get_rect_result(TFV_Id module, TFV_Size* x, TFV_Size* y,
                           TFV_Size* width, TFV_Size* height) {
    return TFV_NOT_IMPLEMENTED;
}
TFV_Result get_string_result(TFV_Id module, TFV_String* result) {
    return TFV_NOT_IMPLEMENTED;
}
}
