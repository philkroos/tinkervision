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
#include <cassert>

#include "api.hh"
#include "logger.hh"

extern "C" {

///
/// Utilities
///
static void copy_std_string(std::string const& str, char cstr[]) {
    assert(str.length() < TV_STRING_SIZE - 1);

    std::strncpy(cstr, str.c_str(), TV_STRING_SIZE - 1);
    // In case str was to long, be sure to nullterminate
    cstr[TV_STRING_SIZE - 1] = '\0';
}

//
// Library functions
//

int16_t tv_camera_available(void) {
    tv::Log("Tinkervision::CameraAvailable");
    return tv::get_api().is_camera_available();
}

int16_t tv_set_framesize(uint16_t width, uint16_t height) {
    tv::Log("Tinkervision::SetFramesize", width, " ", height);
    return tv::get_api().set_framesize(width, height);
}

int16_t tv_start_idle(void) {
    tv::Log("Tinkervision::StartIdle");
    return tv::get_api().start_idle();
}

int16_t tv_effective_frameperiod(uint32_t* frameperiod) {
    tv::Log("Tinkervision::EffectiveFrameperiod");
    *frameperiod = tv::get_api().effective_frameperiod();
    return TV_OK;
}

int16_t tv_get_resolution(uint16_t* width, uint16_t* height) {
    tv::Log("Tinkervision::GetResolution");
    return tv::get_api().resolution(*width, *height);
}

int16_t tv_stop(void) {
    tv::Log("Tinkervision::Stop");

    return tv::get_api().stop();
}

int16_t tv_start(void) {
    tv::Log("Tinkervision::Start");
    return tv::get_api().start();
}

int16_t tv_scene_from_module(int8_t module, int16_t* scene_id) {
    tv::Log("Tinkervision::SceneFromModule", module, " ", scene_id);
    return TV_NOT_IMPLEMENTED;
    // return tv::get_api().scene_start(module, scene_id);
}

int16_t tv_scene_add_module(int16_t scene, int8_t module) {
    tv::Log("Tinkervision::SceneAddModule", scene, " ", module);
    return TV_NOT_IMPLEMENTED;
    // return tv::get_api().add_to_scene(scene, module);
}

int16_t tv_scene_remove(int16_t scene) {
    tv::Log("Tinkervision::SceneRemove", scene);
    return TV_NOT_IMPLEMENTED;
    // return tv::get_api().scene_remove(scene);
}

int16_t tv_quit(void) {
    tv::Log("Tinkervision::Quit");
    return tv::get_api().quit();
}

int16_t tv_request_frameperiod(uint32_t milliseconds) {
    tv::Log("Tinkervision::RequestFrameperiod", milliseconds);
    return tv::get_api().request_frameperiod(milliseconds);
}

char const* tv_result_string(int16_t code) {
    tv::Log("Tinkervision::ResultString", code);
    return tv::get_api().result_string(code);
}

int16_t tv_set_parameter(int8_t module_id, char const* const parameter,
                         int32_t value) {
    tv::Log("Tinkervision::SetParameter", module_id, " ", parameter, " ",
            value);
    return tv::get_api().set_parameter(module_id, parameter, value);
}

int16_t tv_get_parameter(int8_t module_id, char const* const parameter,
                         int32_t* value) {
    tv::Log("Tinkervision::GetParameter", module_id, " ", parameter);
    return tv::get_api().get_parameter(module_id, parameter, value);
}

int16_t tv_module_start(char const* name, int8_t* id) {
    tv::Log("Tinkervision::ModuleStart", name);
    return tv::get_api().module_load(name, *id);
}

int16_t tv_module_restart(int8_t id) {
    tv::Log("Tinkervision::ModuleRestart", id);
    return tv::get_api().module_start(id);
}

int16_t tv_module_stop(int8_t id) {
    tv::Log("Tinkervision::ModuleStop", id);
    return tv::get_api().module_stop(id);
}

int16_t tv_module_remove(int8_t id) {
    tv::Log("Tinkervision::ModuleRemove", id);
    return tv::get_api().module_destroy(id);
}

int16_t tv_module_get_name(int8_t module_id, char name[]) {
    tv::Log("Tinkervision::ModuleGetName", module_id);
    std::string module_name;
    auto err = tv::get_api().module_get_name(module_id, module_name);
    if (err == TV_OK) {
        copy_std_string(module_name, name);
    }
    return err;
}

int16_t tv_module_enumerate_parameters(int8_t module_id,
                                       TV_StringCallback callback,
                                       void* context) {
    tv::Log("Tinkervision::ModuleEnumerateParameters", module_id);
    return tv::get_api().module_enumerate_parameters(module_id, callback,
                                                     context);
}

int16_t tv_libraries_count(uint16_t* count) {
    tv::Log("Tinkervision::LibrariesCount");
    *count = 0;
    tv::get_api().get_libraries_count(*count);
    return TV_OK;
}

int16_t tv_library_name_and_path(uint16_t count, char name[], char path[]) {
    tv::Log("Tinkervision::LibraryNameAndPath");
    std::string sname, spath;
    if (not tv::get_api().library_get_name_and_path(count, sname, spath)) {

        return TV_INVALID_ARGUMENT;
    }
    copy_std_string(sname, name);
    copy_std_string(spath, path);
    return TV_OK;
}

int16_t tv_library_parameter_count(char const* libname, uint16_t* count) {
    tv::Log("Tinkervision::LibraryParameterCount", libname);
    /// \todo Fix types: size_t vs uint16_t
    *count = 0;
    return tv::get_api().library_get_parameter_count(libname,
                                                     *(size_t*)(count));
}

int16_t tv_library_describe_parameter(char const* libname, uint16_t parameter,
                                      char name[], int32_t* min, int32_t* max,
                                      int32_t* def) {
    tv::Log("Tinkervision::LibraryDescribeParameter", libname, " ", parameter);
    std::string sname;
    int16_t err = tv::get_api().library_describe_parameter(
        libname, parameter, sname, *min, *max, *def);
    if (err == TV_OK) {
        copy_std_string(sname, name);
    }

    return err;
}

int16_t tv_libraries_changed_callback(TV_LibrariesCallback callback,
                                      void* context) {
    tv::Log("Tinkervision::LibrariesChangedCallback", (void*)callback, " ",
            context);
    return tv::get_api().libraries_changed_callback(callback, context);
}

int16_t tv_user_module_load_path(char path[]) {
    tv::Log("Tinkervision::UserModuleLoadPath");
    auto spath = tv::get_api().user_module_path();
    copy_std_string(spath, path);
    return TV_OK;
}

int16_t tv_set_user_module_load_path(char const* path) {
    tv::Log("Tinkervision::SetUserModuleLoadPath");
    auto spath = std::string(path);

    if (spath.size() >= TV_STRING_SIZE) {
        return TV_INVALID_ARGUMENT;
    }

    return tv::get_api().set_user_module_load_path(spath);
}

int16_t tv_system_module_load_path(char path[]) {
    tv::Log("Tinkervision::SystemModuleLoadPath");
    auto spath = tv::get_api().system_module_path();
    copy_std_string(spath, path);
    return TV_OK;
}

int16_t tv_remove_all_modules(void) {
    tv::Log("Tinkervision::RemoveAllModules");
    tv::get_api().remove_all_modules();
    return TV_OK;
}

//
// Callbacks
//

int16_t tv_set_callback(int8_t module, TV_Callback callback) {
    tv::Log("Tinkervision::SetCallback", module);
    return tv::get_api().callback_set(module, callback);
}

int16_t tv_enable_default_callback(TV_Callback callback) {
    tv::Log("Tinkervision::EnableDefaultCallback");
    return tv::get_api().callback_default(callback);
}

//
// Accessors for the same data provided by the callbacks
//

int16_t tv_get_result(int8_t module, TV_ModuleResult* result) {
    tv::Log("Tinkervision::GetResult", module);
    return tv::get_api().get_result(module, *result);
}
}
