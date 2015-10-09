/// \file tinkervision.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Defines the public C-interface of the tinkervision library.
///
/// This file is part of Tinkervision - Vision Library for Tinkerforge Redbrick
/// \sa tinkervision.h
/// \sa https://github.com/Tinkerforge/red-brick
///
/// \copyright
///
/// This program is free software; you can redistribute it and/or
/// modify it under the terms of the GNU General Public License
/// as published by the Free Software Foundation; either version 2
/// of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
/// USA.

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

TV_Result tv_camera_available(void) {
    tv::Log("Tinkervision::CameraAvailable");
    return tv::get_api().is_camera_available();
}

TV_Result tv_preselect_framesize(TV_Size width, TV_Size height) {
    tv::Log("Tinkervision::PreselectFramesize", width, " ", height);
    return tv::get_api().preselect_framesize(width, height);
}

TV_Result tv_start_idle(void) {
    tv::Log("Tinkervision::StartIdle");
    return tv::get_api().start_idle();
}

TV_Result tv_get_resolution(TV_Size* width, TV_Size* height) {
    tv::Log("Tinkervision::GetResolution");

    return tv::get_api().resolution(*width, *height);
}

TV_Result tv_stop(void) {
    tv::Log("Tinkervision::Stop");

    return tv::get_api().stop();
}

TV_Result tv_start(void) {
    tv::Log("Tinkervision::Start");
    return tv::get_api().start();
}

TV_Result tv_scene_from_module(TV_Id module, TV_Scene* scene_id) {
    tv::Log("Tinkervision::SceneFromModule", module, " ", scene_id);
    return TV_NOT_IMPLEMENTED;
    // return tv::get_api().scene_start(module, scene_id);
}

TV_Result tv_scene_add_module(TV_Scene scene, TV_Id module) {
    tv::Log("Tinkervision::SceneAddModule", scene, " ", module);
    return TV_NOT_IMPLEMENTED;
    // return tv::get_api().add_to_scene(scene, module);
}

TV_Result tv_scene_remove(TV_Scene scene) {
    tv::Log("Tinkervision::SceneRemove", scene);
    return TV_NOT_IMPLEMENTED;
    // return tv::get_api().scene_remove(scene);
}

TV_Result tv_quit(void) {
    tv::Log("Tinkervision::Quit");
    return tv::get_api().quit();
}

TV_Result tv_set_execution_latency(TV_UInt milliseconds) {
    tv::Log("Tinkervision::SetExecutionLatency", milliseconds);
    return tv::get_api().set_execution_latency_ms(milliseconds);
}

TV_String tv_result_string(TV_Result code) {
    tv::Log("Tinkervision::ResultString", code);
    return tv::get_api().result_string(code);
}

TV_Result tv_set_parameter(TV_Id module_id, TV_String const parameter,
                           TV_Int value) {
    tv::Log("Tinkervision::SetParameter", module_id, " ", parameter, " ",
            value);
    return tv::get_api().set_parameter(module_id, parameter, value);
}

TV_Result tv_get_parameter(TV_Id module_id, TV_String const parameter,
                           TV_Int* value) {
    tv::Log("Tinkervision::GetParameter", module_id, " ", parameter);
    return tv::get_api().get_parameter(module_id, parameter, value);
}

TV_Result tv_module_start(TV_String name, TV_Id* id) {
    tv::Log("Tinkervision::ModuleStart", name);
    return tv::get_api().module_load(name, *id);
}

TV_Result tv_module_restart(TV_Id id) {
    tv::Log("Tinkervision::ModuleRestart", id);
    return tv::get_api().module_start(id);
}

TV_Result tv_module_stop(TV_Id id) {
    tv::Log("Tinkervision::ModuleStop", id);
    return tv::get_api().module_stop(id);
}

TV_Result tv_module_remove(TV_Id id) {
    tv::Log("Tinkervision::ModuleRemove", id);
    return tv::get_api().module_destroy(id);
}

TV_Result tv_module_get_name(TV_Id module_id, TV_CharArray name) {
    tv::Log("Tinkervision::ModuleGetName", module_id);
    std::string module_name;
    auto err = tv::get_api().module_get_name(module_id, module_name);
    if (err == TV_OK) {
        std::strncpy(name, module_name.c_str(), TV_CHAR_ARRAY_SIZE - 1);
        name[TV_CHAR_ARRAY_SIZE - 1] = '\0';
    }
    return err;
}

TV_Result tv_module_enumerate_parameters(TV_Id module_id,
                                         TV_StringCallback callback,
                                         TV_Context context) {
    tv::Log("Tinkervision::ModuleEnumerateParameters", module_id);
    return tv::get_api().module_enumerate_parameters(module_id, callback,
                                                     context);
}

TV_Result tv_enumerate_available_modules(TV_StringCallback callback,
                                         TV_Context context) {
    tv::Log("Tinkervision::EnumerateAvailableModules");
    return tv::get_api().enumerate_available_modules(callback, context);
}

//
// Callbacks
//

TV_Result tv_set_callback(TV_Id module, TV_Callback callback) {
    tv::Log("Tinkervision::SetCallback", module);
    return tv::get_api().callback_set(module, callback);
}

TV_Result tv_enable_default_callback(TV_Callback callback) {
    tv::Log("Tinkervision::EnableDefaultCallback");
    return tv::get_api().callback_default(callback);
}

//
// Accessors for the same data provided by the callbacks
//

TV_Result tv_get_result(TV_Id module, TV_ModuleResult* result) {
    tv::Log("Tinkervision::GetResult", module);
    return tv::get_api().get_result(module, *result);
}
}
