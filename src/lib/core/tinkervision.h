/// \file tinkervision.h
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Declaration of the public C-interface of the tinkervision library.
///
/// This file is part of Tinkervision - Vision Library for Tinkerforge Redbrick
/// \sa api.hh
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

#include "tinkervision_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// GENERAL FUNCTIONS
//

/// Check if the library is valid.  Should be called once before usage.
/// \return
///    - TV_INTERNAL_ERROR if an error occured during construction.
///    - TV_OK if the library is usable.
int16_t tv_valid(void);

int16_t tv_latency_test(void);
int16_t tv_duration_test(uint16_t);

#ifndef DEFAULT_CALL
/// Get the buffered result, if available. If any operation returns
/// #TV_RESULT_BUFFERED, this method can be called until anything else is
/// returned.
/// \return
///    - #TV_RESULT_BUFFERED until the result is available
///    - result of the last buffered op else.
int16_t tv_get_buffered_result(void);
#endif

/// Check if a specific camera device is available.
/// \return
///    - #TV_CAMERA_NOT_AVAILABLE if not.
///    - #TV_OK else.
int16_t tv_camera_id_available(uint8_t id);

/// Select a specific camera.
/// If the specified camera is not available in the system, another one may
/// still be used. This is for the case that multiple cameras are available.
/// If the library is already active with another camera and the selected one
/// would be available too, it will be switched.
/// \return
///    - #TV_CAMERA_NOT_AVAILABLE if the camera with id is not available or
///    the global camera state changed with this method (e.g. open previously,
///    closed now, no matter which id).
///    - #TV_OK else.
int16_t tv_prefer_camera_with_id(uint8_t id);

/// Check if any camera device is available.
/// \return
///    - #TV_CAMERA_NOT_AVAILABLE if not.
///    - #TV_OK else.
int16_t tv_camera_available(void);

/// Pause the Api, deactivating but not disabling every module.  The camera
/// will
/// be released and no further callbacks will be called, but on start(), the
/// Api
/// will be found in the exact same state as left (assuming that the camera
/// can
/// be acquired again).
///     - #TV_OK on success.
///     - #TV_EXEC_THREAD_FAILURE on error.
int16_t tv_stop(void);

/// Restart the API from paused state, initiated through call to stop().
/// \return
///   - #TV_OK if the api was stoppend and is running now.
///   - an error code else; also if the api was already running.
int16_t tv_start(void);

/// Stop all modules and shutdown the api.  This is generally not necessary
/// if
/// the client application terminates in controlled ways.
/// \return TV_OK
int16_t tv_quit(void);

/// Request the resolution of the camera frames.  This can only be called
/// once
/// the camera is active, so in particular, if the resolution needs to be
/// known
/// before a module can be started, start_idle() must be called.
/// \param[out] width
/// \param[out] height
/// \return
///  - #TV_Ok if width and height are valid
///  - #TV_CAMERA_NOT_AVAILABLE else
int16_t tv_get_framesize(uint16_t* width, uint16_t* height);

/// Selects a framesize WxH.
/// This will temporarily stop and restart all active modules.
/// If the requested framesize is not available, the settings will be
/// restored
/// to the last valid settings, if any.  If no module is running, the camera
/// will just be tested.
/// \param[in] width
/// \param[in] height
/// \return
///   - #TV_OK if the settings are ok.
///   - #TV_CAMERA_SETTINGS_FAILED if the settings are ignored.
int16_t tv_set_framesize(uint16_t width, uint16_t height);

/// Set the minimum inverse frame frequency. Vision modules registered
/// and started in the api will be executed sequentially during one
/// execution loop. The execution latency set here is the minimum
/// delay between two loops, i.e. the minimum inverse framerate
/// (frames are grabbed once at the beginning of each loop).
/// \param[in] milliseconds The minimum delay between the beginning of
/// two execution loops.
/// \return TV_OK in any case.
int16_t tv_request_frameperiod(uint32_t milliseconds);

/// Get the effective frameperiod, which can be larger than the frameperiod
/// requested.
/// \param[out] frameperiod Effective, inverse framerate.
/// \return TV_OK.
int16_t tv_effective_frameperiod(uint32_t* frameperiod);

/// Access the currently set user paths prefix.
/// \see tv_set_user_paths_prefix()
/// \param[out] path The user defined path.
/// \return TV_OK
int16_t tv_get_user_paths_prefix(char path[]);

/// Set the user paths prefix from an existing path. The directory has to
/// provide the subdirectories modules (path searched for user modules),
/// frames
/// (default path to store or load frames from) and scripts (path used to
/// load
/// python scripts from).
/// \param[in] path The user defined path. Must not exceed
/// #TV_STRING_SIZE characters
/// \return
///    - #TV_INVALID_ARGUMENT if the string is too long or the path does not
///    exist.
///    - #TV_OK else
int16_t tv_set_user_paths_prefix(char const* path);

/// Access the fixed system module load path.
/// \param[out] path The fixed system path searched for modules.
/// \return TV_OK
int16_t tv_get_system_module_load_path(char path[]);

/// Retrieve the number of loaded libraries.
/// The result can be used with tv_get_module_id().
/// \param[out] count Number of loaded libraries.
/// \return #TV_OK
int16_t tv_get_loaded_libraries_count(uint16_t* count);

/// Retrieve the id of a loaded library.
/// \param[in] library A number \c [0, tv_get_loaded_libraries_count())
/// \param[out] id Id of the library.
/// \return
///    - #TV_INVALID_ARGUMENT if the argument is out of range.
///    - #TV_OK else
int16_t tv_get_module_id(int16_t library, int8_t* id);

/// Get a string representation of a result code.
/// \param[in] code one of the TV_* values.
/// \return The associated string value.
char const* tv_result_string(int16_t code);

//
// LIBRARY INFORMATION FUNCTIONS
//

/// Get the number of currently available libraries.
/// This can be used to iterate through all libraries using
/// tv_library_name_and_path().
/// \see tv_get_loaded_libraries_count() to only get the number of loaded
/// libs.
/// \return #TV_OK
/// \param[out] count Number of libraries found in both system and user
/// paths.
int16_t tv_libraries_count(uint16_t* count);

/// Get the name and load path of an available library.
/// \param[in] count A number smaller then tv_libraries_count().
/// \param[out]  A number smaller then tv_libraries_count().
/// \param[out] path Either "system" or "user". Get the actual values with
/// tv_user_module_load_path() and tv_system_module_load_path(),
/// respectively.
/// \return
///    - #TV_OK if name and path are valid.
///    - #TV_INVALID_ARGUMENT if an error occured, probably count was out of
///    range.
int16_t tv_library_name_and_path(uint16_t count, char name[], char path[]);

/// Get the number of parameters a library supports.
/// \param[in] libname Name of the module, i.e. filename w/o extension.
/// \param[out] count Number of supported parameters.
/// \note count is beeing set to 0 if libname is not available
/// \return
///    - #TV_INVALID_ARGUMENT: The library is not available.
///    - #TV_OK else
int16_t tv_library_parameters_count(char const* libname, uint16_t* count);

/// Get the properties of a parameter from a library.
/// \param[in] libname Name of the module, i.e. filename w/o extension.
/// \param[in] parameter A number < tv_library_parameter_count()
/// \param[out] name Parameter identifier
/// \param[out] type Parameter type: 0 numeric, 1 string.
/// \param[out] string Parameter value if type = string.
/// \param[out] min Parameter minimum value if type = numeric.
/// \param[out] max Parameter maximum value if type = numeric.
/// \param[out] def Parameter default value if type = numeric.
/// \return False if the library is not available or number is out of range.
///    - #TV_INVALID_ARGUMENT: The library is not available or parameter
///    exceeds
///    the available number of parameters.
///    - #TV_OK else
/// \note All outgoing parameters are beeing set to 0 if the result is not
/// TV_OK.
int16_t tv_library_parameter_describe(char const* libname, uint16_t parameter,
                                      char name[], uint8_t* type, int32_t* min,
                                      int32_t* max, int32_t* def);

//
// MODULE HANDLING FUNCTIONS
//

/// Starts a dummy module keeping the Api up and running even if no 'real'
/// module
/// is active.  This can be used to block the camera or if the resolution
/// has to
/// be known before any module is running.  Subsequent calls will not start
/// another dummy.  The Dummy, once started, can only be quit by calling
/// quit().
/// \return
///   - #TV_OK if the dummy module was started or already running.
///   - An error code if the module failed to load.
int16_t tv_start_idle(void);

/// Start a vision module identified by its library name.
/// The requested module will be loaded and started if it is found in one of
/// the
/// available library search paths #SYS_MODULE_LOAD_PATH or
/// #ADD_MODULE_LOAD_PATH.  On success, it will be assigned a unique id.
/// \param[in] name The name of the requested module, which is the exact
/// filename of the  corresponding library without extension.
/// \param[out] id On success, the loaded module will be accessible with
/// this
/// id.
/// \todo Add method to enable parallel module start instead of currently
/// fixed
/// sequential.
/// \return
///    - #TV_INTERNAL_ERROR if an id clash occured. This is an open but
///      unlikely bug.
///    - #TV_CAMERA_NOT_AVAILABLE if the camera could not be opened.
///    - #TV_MODULE_INITIALIZATION_FAILED The module could not be loaded.
///    Maybe
///      an invalid name passed.
///    - #TV_OK fine, module loaded and active.
int16_t tv_module_start(char const* name, int8_t* id);

/// Disable a module without removing it.
/// A disabled module won't be executed, but it is still available for
/// configuration or reactivation. The associated camera will be released if
/// it
/// is not used by other modules.
/// \see tv_module_restart()
/// \param[in] id Id of the module to be stopped.
/// \return
///    - #TV_INVALID_ID if no such module exists.
///    - #TV_OK else, the module is inactive.
int16_t tv_module_stop(int8_t id);

/// Restart a module that has been stopped with module_stop().
/// \param[in] id Id of the module to be restarted.
/// \return
///    - #TV_INVALID_ID if no module is registered with
///      the given id.
///    - #TV_CAMERA_ACQUISATION_FAILED if the camera is not available
///    - #TV_OK iff the module is running.
int16_t tv_module_restart(int8_t id);

/// Force execution of a module now.
/// Now means, as soon as possible. The currently executed module, if any,
/// won't
/// be stopped, but the main execution loop will be <paused> immediately
/// after
/// new frame will be acquired and the requested module will execute.  If id
/// is
/// not available, the loop won't be stopped. The module will receive the
/// current frame.
/// \param[in] id Id of the module to execute.
/// \return
///    - #TV_CAMERA_NOT_AVAILABLE camera not open
///    - #TV_INVALID_ID module not found
///    - #TV_INVALID_OK module will be executed.
int16_t tv_module_run_now(int8_t id);

/// Do the same as tv_module_run_now(), but acquire a new frame first.
/// This will also request the next frame and restart the main execution
/// loop
/// after id has been executed, since all sequenced modules are guaranteed
/// to
/// receive the same frame during one loop.
/// \param[in] id Id of the module to execute.
/// \return
///    - #TV_CAMERA_NOT_AVAILABLE camera not open
///    - #TV_INVALID_ID module not found
///    - #TV_INVALID_OK module will be executed and the main execution loop
///    will
///    restart.
int16_t tv_module_run_now_new_frame(int8_t id);

/// Check if a loaded module is active, i.e. actually running.
/// \param[in] id Id of an active module.
/// \param[out] active Boolean value, 0 if inactive.
/// \return
///    - #TV_INVALID_ID if no such module is loaded.
///    - #TV_OK else
int16_t tv_module_is_active(int8_t id, uint8_t* active);

/// Deactivate and remove a module.
/// The id of a removed module is invalid afterwards.
/// \param[in] id Id of the module to be restarted.
/// \return
///    - #TV_NOT_IMPLEMENTED if scenes are active.
///    - #TV_INVALID_ID if the module does not exist.
///    - #TV_OK if removal succeeded.
int16_t tv_module_remove(int8_t id);

/// Get the name of a loaded module.
/// \param[in] id The id of the module.
/// \param[out] name The name, which is exactly the name that would be
/// passed to
/// module_start().
/// \return
///    - #TV_INVALID_ID If no such module exists.
///    - #TV_OK else, name will be valid.
int16_t tv_module_get_name(int8_t id, char name[]);

/// Get the result of the latest execution of a given module.
/// \param[in] id The module in question.
/// \param[out] result The latest result of the module, if any.
/// \todo What else?
/// \return TV_INVALID_ID if no such module exists.
/// \return TV_OK else.
int16_t tv_module_get_result(int8_t module, TV_ModuleResult* result);

/// Disable, remove and destroy all modules.
/// \return #TV_OK
int16_t tv_remove_all_modules(void);

//
// MODULE PARAMATERS FUNCTIONS
//

/// Get the identifiers of all parameters of a loaded module.
/// \param[in] id The id of the module.
/// \param[in] callback The given method will be called for each parameter's
/// name.
/// \param[in] context A pointer to something, which can be used to
/// differentiate between several instances of #char const*Callback.
/// \return
///    - #TV_INVALID_ID If no such module exists.
///    - #TV_OK else, callback will be called with the id of the module, one
///      parameter name per time, and \c context. After all parameter names
///      have
///      been passed, the callback receives an id of \c 0 and no further
///      callbacks will be received.
/// \deprecated Use tv_library_parameter_count() and
/// tv_library_describe_parameter().
int16_t tv_module_enumerate_parameters(int8_t module_id,
                                       TV_StringCallback callback,
                                       void* context);

/// Return the current value of a modules parameter.
/// \param[in] module_id The id of the module in question.
/// \param[in] parameter Name of the parameter in question.
/// \param[out] value Current value of parameter.
/// \return
///   - #TV_INVALID_ID if no module exists with module_id.
///   - #TV_MODULE_NO_SUCH_PARAMETER if the module does not support
///   parameter.
///   - #TV_OK else.
int16_t tv_module_get_numerical_parameter(int8_t module_id,
                                          char const* const parameter,
                                          int32_t* value);

/// Parameterize a module.
/// \param[in] module_id Id of the module to be parameterized.
/// \param[in] parameter name of the parameter to be set.
/// \param[in] value Value to be set for parameter.
/// \return
///   - #TV_MODULE_NO_SUCH_PARAMETER if the module does not support
///   parameter.
///   - #TV_MODULE_ERROR_SETTING_PARAMETER a module internal error during
/// setting
/// of the parameter.
///   - #TV_INVALID_ID if no module exists with module_id
///   - #TV_OK else.
int16_t tv_module_set_numerical_parameter(int8_t module_id,
                                          char const* const parameter,
                                          int32_t value);

int16_t tv_module_get_string_parameter(int8_t module_id,
                                       char const* const parameter,
                                       char value[]);

int16_t tv_module_set_string_parameter(int8_t module_id,
                                       char const* const parameter,
                                       char const* value);

//
// SCENE HANDLING FUNCTIONS
//

/// Start a new scene given a loaded module.
/// \param[in] id Id of a loaded module.
/// \param[out] scene Id of the scene if it was created successfully.
/// \return
///    - #TV_NOT_IMPLEMENTED Currently. (If the given module will be
///    removed)
///    - #TV_INVALID_ID (If no module with id exists)
///    - #TV_OK (if the scene was started successfully)
///    - (else, an error during scene creation occured.)
int16_t tv_scene_from_module(int8_t id, int16_t* scene);

/// Add a module to an existing scene.
/// \param[in] scene Id of the scene.
/// \param[in] id Id of the module.
/// \return
///    - #TV_NOT_IMPLEMENTED id currently.
int16_t tv_scene_add_module(int16_t scene, int8_t module);

/// Remove a scene without affecting the associated modules.
/// \param[in] scene Id of the scene.
/// \return
///    - #TV_NOT_IMPLEMENTED id currently.
int16_t tv_scene_remove(int16_t scene);

//
// CALLBACKS
//

/// Set a callback to the result of a specific module.
/// The given callback will be called after each execution of the specified
/// module, provided it has produced a result.
/// \param[in] id The module.
/// \param[in] callback The function to be called for each result.
/// \return
///    - #TV_INVALID_ID if no such module exists.
///    - #TV_GLOBAL_CALLBACK_ACTIVE if a default callback has been set
///       previously with enable_default_callback()
///    - #TV_INTERNAL_ERROR That's a bug, please report.
///    - #TV_OK if the callback has been set successfully.
/// \deprecated This method is not activated in the Red-Brick interface
/// since it
/// does not make sense there.  We can't activate callbacks per module with
/// the
/// generated api.  This method will probably be removed to focus on the
/// Red-Brick.
int16_t tv_callback_set(int8_t id, TV_Callback callback);

/// Enable a default callback which will be called for each result of every
/// module.
/// \param[in] callback The default callback.
int16_t tv_callback_enable_default(TV_Callback callback);

/// Notify the user if a library path changes.
/// \param[in] callback The given method will be called when a loadable
/// module
/// is being created or deleted in one of the available loadpaths.  The
/// callback
/// will receive the name and path of the library and the event, which can
/// be
/// "create" (1) or "remove" (-1).
/// \param[in] context A pointer to something.
/// \return
///    - #TV_OK always.
int16_t tv_callback_libraries_changed_set(TV_LibrariesCallback callback,
                                          void* context);

#ifdef __cplusplus
}
#endif
