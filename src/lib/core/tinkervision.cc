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
#include <atomic>
#include <thread>

#include "api.hh"
#include "logger.hh"

extern "C" {

#ifndef DEFAULT_CALL
static std::atomic_int tv_buffered_result{
    TV_OK};  ///< If an op takes too long, buffer for the
             /// result, retrievable with get_buffered_result()
static std::atomic_flag tv_buffer_flag{
    ATOMIC_FLAG_INIT};  ///< Used to signal an operation is finished

#define LOW_LATENCY_CALL(code)                                                \
    tv_buffered_result = TV_RESULT_BUFFERED;                                  \
    if (tv_buffer_flag.test_and_set()) {                                      \
        return tv_buffered_result;                                            \
    }                                                                         \
    std::thread([&](void) {                                                   \
        tv_buffered_result.store(code);                                       \
        tv_buffer_flag.clear();                                               \
                }).detach();                                                  \
    bool set(true);                                                           \
    for (uint8_t i = 0; i < GRAINS and (set = tv_buffer_flag.test_and_set()); \
         ++i) {                                                               \
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_GRAIN));  \
    }                                                                         \
    if (not set) {                                                            \
        tv_buffer_flag.clear(); /* set in for-loop */                         \
    }                                                                         \
    return static_cast<int16_t>(tv_buffered_result.load());
#else
#define LOW_LATENCY_CALL(code) return code
#endif

//
// Utilities
//
static void copy_std_string(std::string const& str, char cstr[]) {
    assert(str.length() < TV_STRING_SIZE - 1);

    std::strncpy(cstr, str.c_str(), TV_STRING_SIZE - 1);
    // In case str was to long, be sure to nullterminate
    cstr[TV_STRING_SIZE - 1] = '\0';
}

//
// GENERAL FUNCTIONS
//

int16_t tv_valid(void) {
    return tv::get_api().valid() ? TV_OK : TV_INTERNAL_ERROR;
}

int16_t tv_latency_test(void) {
    tv::Log("Tinkervision::LatencyTest:");

    LOW_LATENCY_CALL([](void) { return TV_OK; }());
}

int16_t tv_duration_test(uint16_t milliseconds) {
    tv::Log("Tinkervision::LatencyTest", milliseconds);
    LOW_LATENCY_CALL([&milliseconds](void) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        return TV_OK;
    }());
}

#ifndef DEFAULT_CALL
/// Get the buffered result, if available.
/// \return
///    - #TV_RESULT_BUFFERED until the result is available
///    - result of the last buffered op else.
int16_t tv_get_buffered_result(void) {
    return static_cast<int16_t>(tv_buffered_result.load());
}
#endif

int16_t tv_camera_available(void) {
    tv::Log("Tinkervision::CameraAvailable:");
    return tv::get_api().is_camera_available() ? TV_OK
                                               : TV_CAMERA_NOT_AVAILABLE;
}

int16_t tv_camera_id_available(uint8_t id) {
    tv::Log("Tinkervision::CameraIdAvailable", id);
    return tv::get_api().is_camera_available(id) ? TV_OK
                                                 : TV_CAMERA_NOT_AVAILABLE;
}

int16_t tv_prefer_camera_with_id(uint8_t id) {
    tv::Log("Tinkervision::PreferCameraWithId", id);
    return tv::get_api().prefer_camera_with_id(id) ? TV_OK
                                                   : TV_CAMERA_NOT_AVAILABLE;
}

int16_t tv_stop(void) {
    tv::Log("Tinkervision::Stop");

    return tv::get_api().stop();
}

int16_t tv_start(void) {
    tv::Log("Tinkervision::Start");
    return tv::get_api().start();
}

int16_t tv_quit(void) {
    tv::Log("Tinkervision::Quit");

#ifndef DEFAULT_CALL
    // Wait for any running operations to finish
    while (tv_buffer_flag.test_and_set())
        ;
    tv_buffer_flag.clear();
#endif

    return tv::get_api().quit();
}

int16_t tv_get_framesize(uint16_t* width, uint16_t* height) {
    tv::Log("Tinkervision::GetResolution");
    return tv::get_api().resolution(*width, *height);
}

int16_t tv_set_framesize(uint16_t width, uint16_t height) {
    tv::Log("Tinkervision::SetFramesize", width, " ", height);
    return tv::get_api().set_framesize(width, height);
}

int16_t tv_request_frameperiod(uint32_t milliseconds) {
    tv::Log("Tinkervision::RequestFrameperiod", milliseconds);
    return tv::get_api().request_frameperiod(milliseconds);
}

int16_t tv_effective_frameperiod(uint32_t* frameperiod) {
    tv::Log("Tinkervision::EffectiveFrameperiod");
    *frameperiod = tv::get_api().effective_frameperiod();
    return TV_OK;
}

int16_t tv_get_user_paths_prefix(char path[]) {
    tv::Log("Tinkervision::UserGetPathsPrefix:");
    copy_std_string(tv::get_api().user_paths_prefix(), path);
    return TV_OK;
}

int16_t tv_set_user_paths_prefix(char const* path) {
    tv::Log("Tinkervision::SetUserPathsPrefix");
    auto spath = std::string(path);

    if (spath.size() >= TV_STRING_SIZE) {
        return TV_INVALID_ARGUMENT;
    }

    return tv::get_api().set_user_paths_prefix(spath);
}

int16_t tv_get_system_module_load_path(char path[]) {
    tv::Log("Tinkervision::SystemModuleLoadPath");
    auto& spath = tv::get_api().system_module_path();
    copy_std_string(spath, path);
    return TV_OK;
}

int16_t tv_get_loaded_libraries_count(uint16_t* count) {
    tv::Log("Tinkervision::GetLoadedLibrariesCount:");
    *count = tv::get_api().loaded_libraries_count();
    return TV_OK;
}

int16_t tv_get_module_id(int16_t library, int8_t* id) {
    tv::Log("Tinkervision::GetModuleId", library);
    return tv::get_api().module_id(library, *id);
}

char const* tv_result_string(int16_t code) {
    tv::Log("Tinkervision::ResultString", code);
    return tv::get_api().result_string(code);
}

//
// LIBRARY INFORMATION FUNCTIONS
//

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

int16_t tv_library_parameters_count(char const* libname, uint16_t* count) {
    tv::Log("Tinkervision::LibraryParameterCount", libname);
    /// \todo Fix types: size_t vs uint16_t
    *count = 0;
    return tv::get_api().library_get_parameter_count(libname, *count);
}

int16_t tv_library_parameter_describe(char const* libname, uint16_t parameter,
                                      char name[], uint8_t* type, int32_t* min,
                                      int32_t* max, int32_t* def) {
    tv::Log("Tinkervision::LibraryDescribeParameter", libname, " ", parameter);
    std::string sname;
    int16_t err = tv::get_api().library_describe_parameter(
        libname, parameter, sname, *type, *min, *max, *def);
    if (err == TV_OK) {
        copy_std_string(sname, name);
    }

    return err;
}

//
// MODULE HANDLING FUNCTIONS
//

int16_t tv_start_idle(void) {
    tv::Log("Tinkervision::StartIdle");
    return tv::get_api().start_idle();
}

int16_t tv_module_start(char const* name, int8_t* id) {
    tv::Log("Tinkervision::ModuleStart", name);
    //// \todo Let user specify path (system or user)
    return tv::get_api().module_load(name, *id);
}

int16_t tv_module_stop(int8_t id) {
    tv::Log("Tinkervision::ModuleStop", id);
    LOW_LATENCY_CALL(tv::get_api().module_stop(id));
}

int16_t tv_module_restart(int8_t id) {
    tv::Log("Tinkervision::ModuleRestart", id);
    LOW_LATENCY_CALL(tv::get_api().module_start(id));
}

int16_t tv_module_run_now(int8_t id) {
    tv::Log("Tinkervision::ModuleRestart", id);
    LOW_LATENCY_CALL(tv::get_api().module_run_now(id));
}

int16_t tv_module_run_now_new_frame(int8_t id) {
    tv::Log("Tinkervision::ModuleRestart", id);
    LOW_LATENCY_CALL(tv::get_api().module_run_now_new_frame(id));
}

int16_t tv_module_is_active(int8_t id, uint8_t* active) {
    bool bactive;
    tv::Log("Tinkervision::ModuleIsActive", id);
    auto result = tv::get_api().module_is_active(id, bactive);
    *active = static_cast<uint8_t>(bactive);
    return result;
}

int16_t tv_module_remove(int8_t id) {
    tv::Log("Tinkervision::ModuleRemove", id);
    LOW_LATENCY_CALL(tv::get_api().module_destroy(id));
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

int16_t tv_module_get_result(int8_t module, TV_ModuleResult* result) {
    tv::Log("Tinkervision::GetResult", module);
    return tv::get_api().get_result(module, *result);
}

int16_t tv_remove_all_modules(void) {
    tv::Log("Tinkervision::RemoveAllModules");
    LOW_LATENCY_CALL([](void) {
        tv::get_api().remove_all_modules();
        return TV_OK;
    }());
}

//
// MODULE PARAMATERS FUNCTIONS
//

int16_t tv_module_enumerate_parameters(int8_t module_id,
                                       TV_StringCallback callback,
                                       void* context) {
    tv::Log("Tinkervision::ModuleEnumerateParameters", module_id);
    return tv::get_api().module_enumerate_parameters(module_id, callback,
                                                     context);
}

int16_t tv_module_get_numerical_parameter(int8_t module_id,
                                          char const* const parameter,
                                          int32_t* value) {
    tv::Log("Tinkervision::GetParameter", module_id, " ", parameter);
    return tv::get_api().get_parameter(module_id, parameter, *value);
}

int16_t tv_module_set_numerical_parameter(int8_t module_id,
                                          char const* const parameter,
                                          int32_t value) {
    tv::Log("Tinkervision::SetParameter", module_id, " ", parameter, " ",
            value);
    return tv::get_api().set_parameter(module_id, parameter, value);
}

int16_t tv_module_get_string_parameter(int8_t module_id,
                                       char const* const parameter,
                                       char value[]) {
    tv::Log("Tinkervision::GetParameter", module_id, " ", parameter);
    std::string svalue;
    auto result = tv::get_api().get_parameter(module_id, parameter, svalue);
    if (result == TV_OK) {
        copy_std_string(svalue, value);
    }
    return result;
}

int16_t tv_module_set_string_parameter(int8_t module_id,
                                       char const* const parameter,
                                       char const* value) {
    tv::Log("Tinkervision::SetParameter", module_id, " ", parameter, " ",
            value);
    auto svalue = std::string(value);
    return tv::get_api().set_parameter<std::string>(module_id, parameter,
                                                    svalue);
}

//
// SCENE HANDLING FUNCTIONS
//

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

//
// CALLBACKS
//

int16_t tv_callback_set(int8_t module, TV_Callback callback) {
    tv::Log("Tinkervision::SetCallback", module);
    return tv::get_api().callback_set(module, callback);
}

int16_t tv_callback_enable_default(TV_Callback callback) {
    tv::Log("Tinkervision::EnableDefaultCallback");
    return tv::get_api().callback_default(callback);
}

int16_t tv_callback_libraries_changed_set(TV_LibrariesCallback callback,
                                          void* context) {
    tv::Log("Tinkervision::LibrariesChangedCallback", (void*)callback, " ",
            context);
    return tv::get_api().libraries_changed_callback(callback, context);
}
}
