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

#include "tinkervision.h"

#include <map>
#include <string>
#include <cstring>

#include "api.hh"
#include "logger.hh"

extern "C" {

//
// General library functions
//

TFV_Result camera_available(void) {
    tfv::Log("Tinkervision::CameraAvailable");
    return tfv::get_api().is_camera_available();
}

TFV_Result preselect_framesize(TFV_Size width, TFV_Size height) {
    tfv::Log("Tinkervision::PreselectFramesize", width, " ", height);
    return tfv::get_api().preselect_framesize(width, height);
}

TFV_Result start_idle(void) {
    tfv::Log("Tinkervision::StartIdle");
    return tfv::get_api().start_idle();
}

TFV_Result get_resolution(TFV_Size* width, TFV_Size* height) {
    tfv::Log("Tinkervision::GetResolution");

    return tfv::get_api().resolution(*width, *height);
}

TFV_Result stop(void) {
    tfv::Log("Tinkervision::Stop");

    return tfv::get_api().stop();
}

TFV_Result start(void) {
    tfv::Log("Tinkervision::Start");
    return tfv::get_api().start();
}

TFV_Result scene_from_module(TFV_Id module, TFV_Scene* scene_id) {
    tfv::Log("Tinkervision::SceneFromModule", module, " ", scene_id);
    return TFV_NOT_IMPLEMENTED;
    // return tfv::get_api().scene_start(module, scene_id);
}

TFV_Result scene_add_module(TFV_Scene scene, TFV_Id module) {
    tfv::Log("Tinkervision::SceneAddModule", scene, " ", module);
    return TFV_NOT_IMPLEMENTED;
    // return tfv::get_api().add_to_scene(scene, module);
}

TFV_Result scene_remove(TFV_Scene scene) {
    tfv::Log("Tinkervision::SceneRemove", scene);
    return TFV_NOT_IMPLEMENTED;
    // return tfv::get_api().scene_remove(scene);
}

TFV_Result quit(void) {
    tfv::Log("Tinkervision::Quit");
    return tfv::get_api().quit();
}

TFV_Result set_execution_latency(TFV_UInt milliseconds) {
    tfv::Log("Tinkervision::SetExecutionLatency", milliseconds);
    return tfv::get_api().set_execution_latency_ms(milliseconds);
}

TFV_String result_string(TFV_Result code) {
    tfv::Log("Tinkervision::ResultString", code);
    return tfv::get_api().result_string(code);
}

TFV_Result set_parameter(TFV_Id module_id, TFV_String const parameter,
                         TFV_Int value) {
    tfv::Log("Tinkervision::SetParameter", module_id, " ", parameter, " ",
             value);
    return tfv::get_api().set_parameter(module_id, parameter, value);
}

TFV_Result get_parameter(TFV_Id module_id, TFV_String const parameter,
                         TFV_Int* value) {
    tfv::Log("Tinkervision::GetParameter", module_id, " ", parameter);
    return tfv::get_api().get_parameter(module_id, parameter, value);
}

TFV_Result module_start(TFV_String name, TFV_Id* id) {
    tfv::Log("Tinkervision::ModuleStart", name);
    return tfv::get_api().module_load(name, *id);
}

TFV_Result module_restart(TFV_Id id) {
    tfv::Log("Tinkervision::ModuleRestart", id);
    return tfv::get_api().module_start(id);
}

TFV_Result module_stop(TFV_Id id) {
    tfv::Log("Tinkervision::ModuleStop", id);
    return tfv::get_api().module_stop(id);
}

TFV_Result module_remove(TFV_Id id) {
    tfv::Log("Tinkervision::ModuleRemove", id);
    return tfv::get_api().module_destroy(id);
}

TFV_Result module_get_name(TFV_Id module_id, TFV_CharArray name) {
    tfv::Log("Tinkervision::ModuleGetName", module_id);
    std::string module_name;
    auto err = tfv::get_api().module_get_name(module_id, module_name);
    if (err == TFV_OK) {
        std::strncpy(name, module_name.c_str(), TFV_CHAR_ARRAY_SIZE - 1);
        name[TFV_CHAR_ARRAY_SIZE - 1] = '\0';
    }
    return err;
}

TFV_Result module_enumerate_parameters(TFV_Id module_id,
                                       TFV_StringCallback callback,
                                       TFV_Context context) {
    tfv::Log("Tinkervision::ModuleEnumerateParameters", module_id);
    return tfv::get_api().module_enumerate_parameters(module_id, callback,
                                                      context);
}

TFV_Result enumerate_available_modules(TFV_StringCallback callback,
                                       TFV_Context context) {
    tfv::Log("Tinkervision::EnumerateAvailableModules");
    return tfv::get_api().enumerate_available_modules(callback, context);
}

//
// Callbacks
//

TFV_Result set_callback(TFV_Id module, TFV_Callback callback) {
    tfv::Log("Tinkervision::SetCallback", module);
    return tfv::get_api().callback_set(module, callback);
}

TFV_Result enable_default_callback(TFV_Callback callback) {
    tfv::Log("Tinkervision::EnableDefaultCallback");
    return tfv::get_api().callback_default(callback);
}

//
// Accessors for the same data provided by the callbacks
//

TFV_Result get_result(TFV_Id module, TFV_ModuleResult* result) {
    tfv::Log("Tinkervision::GetResult", module);
    return tfv::get_api().get_result(module, *result);
}
}
