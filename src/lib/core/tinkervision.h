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

/// Checks if the camera device is available.
/// \return
///    - TV_CAMERA_NOT_AVAILABLE if not.
///    - TV_OK else.
TV_Result tv_camera_available(void);

/// Selects a framesize WxH.
/// This will temporarily stop and restart all active modules.
/// If the requested framesize is not available, the settings will be restored
/// to the last valid settings, if any.  If no module is running, the camera
/// will just be tested.
/// \param[in] width
/// \param[in] height
/// \return
///   - #TV_OK if the settings are ok.
///   - #TV_CAMERA_SETTINGS_FAILED if the settings are ignored.
TV_Result tv_set_framesize(TV_Size width, TV_Size height);

/// Starts a dummy module keeping the Api up and running even if no 'real'
/// module
/// is active.  This can be used to block the camera or if the resolution has to
/// be known before any module is running.  Subsequent calls will not start
/// another dummy.  The Dummy, once started, can only be quit by calling quit().
/// \return
///   - #TV_OK if the dummy module was started or already running.
///   - An error code if the module failed to load.
TV_Result tv_start_idle(void);

/// Introduce a delay into the execution to save processing power.  All
/// vision-modules registered and started in the api will be executed
/// sequentially during one execution loop. The execution latency set here is
/// the
/// minimum delay between two loops, i.e. a sort of minimum inverse framerate
/// (frames are grabbed once at the beginning of each loop).
/// \param[in] milliseconds The minimum delay between the beginning of two
/// execution loops.
/// \return TV_OK in any case. There is however a minimum internal latency of
/// currently 20ms which won't be ignored.
TV_Result tv_set_execution_latency(TV_UInt milliseconds);

/// Get the effective framerate, which can be worse than the framerate
/// requested.
/// \param[out] framerate Effective, inverse framerate.
/// \return TV_OK.
TV_Result tv_effective_inv_framerate(double* framerate);

/// Request the resolution of the camera frames.  This can only be called once
/// the camera is active, so in particular, if the resolution needs to be known
/// before a module can be started, start_idle() must be called.
/// \param[out] width
/// \param[out] height
/// \return
///  - #TV_Ok if width and height are valid
///  - #TV_CAMERA_NOT_AVAILABLE else
TV_Result tv_get_resolution(TV_Size* width, TV_Size* height);

/// Pause the Api, deactivating but not disabling every module.  The camera will
/// be released and no further callbacks will be called, but on start(), the Api
/// will be found in the exact same state as left (assuming that the camera can
/// be acquired again).
///     - #TV_OK on success.
///     - #TV_EXEC_THREAD_FAILURE on error.
TV_Result tv_stop(void);

/// Restart the API from paused state, initiated through call to stop().
/// \return
///   - #TV_OK if the api was stoppend and is running now.
///   - an error code else; also if the api was already running.
TV_Result tv_start(void);

/// Stop all modules and shutdown the api.  This is generally not necessary if
/// the client application terminates in controlled ways.
/// \return TV_OK
TV_Result tv_quit(void);

/// Parameterize a module.
/// \param[in] module_id Id of the module to be parameterized.
/// \param[in] parameter name of the parameter to be set.
/// \param[in] value Value to be set for parameter.
/// \return
///   - #TV_MODULE_NO_SUCH_PARAMETER if the module does not support parameter.
///   - #TV_MODULE_ERROR_SETTING_PARAMETER a module internal error during
/// setting
/// of the parameter.
///   - #TV_INVALID_ID if no module exists with module_id
///   - #TV_OK else.
TV_Result tv_set_parameter(TV_Id module_id, TV_String const parameter,
                           TV_Int value);

/// Return the current value of a modules parameter.
/// \param[in] module_id The id of the module in question.
/// \param[in] parameter Name of the parameter in question.
/// \param[out] value Current value of parameter.
/// \return
///   - #TV_INVALID_ID if no module exists with module_id.
///   - #TV_MODULE_NO_SUCH_PARAMETER if the module does not support parameter.
///   - #TV_OK else.
TV_Result tv_get_parameter(TV_Id module_id, TV_String const parameter,
                           TV_Long* value);

/// Start a vision module identified by its library name.
/// The requested module will be loaded and started if it is found in one of the
/// available library search paths #SYS_MODULE_LOAD_PATH or
/// #ADD_MODULE_LOAD_PATH.  On success, it will be assigned a unique id.
/// \param[in] name The name of the requested module, which is the exact
/// filename of the  corresponding library without extension.
/// \param[out] id On success, the loaded module will be accessible with this
/// id.
/// \return
///    - #TV_INTERNAL_ERROR if an id clash occured. This is an open but
///      unlikely bug.
///    - #TV_CAMERA_NOT_AVAILABLE if the camera could not be opened.
///    - #TV_MODULE_INITIALIZATION_FAILED The module could not be loaded. Maybe
///      an invalid name passed.
///    - #TV_OK fine, module loaded and active.
TV_Result tv_module_start(TV_String name, TV_Id* id);

/// Disable a module without removing it.
/// A disabled module won't be executed, but it is still available for
/// configuration or reactivation. The associated camera will be released if it
/// is not used by other modules.
/// \see tv_module_restart()
/// \param[in] id Id of the module to be stopped.
/// \return
///    - #TV_INVALID_ID if no such module exists.
///    - #TV_OK else, the module is inactive.
TV_Result tv_module_stop(TV_Id id);

/// Restart a module that has been stopped with module_stop().
/// \param[in] id Id of the module to be restarted.
/// \return
///    - #TV_INVALID_ID if no module is registered with
///      the given id.
///    - #TV_CAMERA_ACQUISATION_FAILED if the camera is not available
///    - #TV_OK iff the module is running.
TV_Result tv_module_restart(TV_Id id);

/// Deactivate and remove a module.
/// The id of a removed module is invalid afterwards.
/// \param[in] id Id of the module to be restarted.
/// \return
///    - #TV_NOT_IMPLEMENTED if scenes are active.
///    - #TV_INVALID_ID if the module does not exist.
///    - #TV_OK if removal succeeded.
TV_Result tv_module_remove(TV_Id id);

/// Get the name of a loaded module.
/// \param[in] id The id of the module.
/// \param[out] name The name, which is exactly the name that would be passed to
/// module_start().
/// \return
///    - #TV_INVALID_ID If no such module exists.
///    - #TV_OK else, name will be valid.
TV_Result tv_module_get_name(TV_Id id, TV_CharArray name);

/// Get the identifiers of all parameters of a loaded module.
/// \param[in] id The id of the module.
/// \param[in] callback The given method will be called for each parameter's
/// name.
/// \param[in] context A pointer to something, which can be used to
/// differentiate between several instances of #TV_StringCallback.
/// \return
///    - #TV_INVALID_ID If no such module exists.
///    - #TV_OK else, callback will be called with the id of the module, one
///      parameter name per time, and \c context. After all parameter names have
///      been passed, the callback receives an id of \c 0 and no further
///      callbacks will be received.
/// \deprecated Use tv_library_parameter_count() and
/// tv_library_describe_parameter().
TV_Result tv_module_enumerate_parameters(TV_Id module_id,
                                         TV_StringCallback callback,
                                         TV_Context context);

/// Get the number of parameters a library supports.
/// \param[in] libname Name of the module, i.e. filename w/o extension.
/// \param[out] count Number of supported parameters.
/// \note count is beeing set to 0 if libname is not available
/// \return
///    - #TV_INVALID_ARGUMENT: The library is not available.
///    - #TV_OK else
TV_Result tv_library_parameter_count(TV_String libname, TV_Size* count);

/// Get the properties of a parameter from a library.
/// \param[in] libname Name of the module, i.e. filename w/o extension.
/// \param[in] parameter A number < tv_library_parameter_count()
/// \param[out] name Parameter identifier
/// \param[out] min Parameter minimum value
/// \param[out] max Parameter maximum value
/// \param[out] def Parameter default value
/// \return False if the library is not available or number is out of range.
///    - #TV_INVALID_ARGUMENT: The library is not available or parameter exceeds
///    the available number of parameters.
///    - #TV_OK else
/// \note All outgoing parameters are beeing set to 0 if the result is not
/// TV_OK.
TV_Result tv_library_describe_parameter(TV_String libname, TV_Size parameter,
                                        TV_CharArray name, TV_Long* min,
                                        TV_Long* max, TV_Long* def);

/// Get the names of all available modules.
/// \param[in] callback The given method will be called for each loadable
/// module.
/// \param[in] context A pointer to something, which can be used to
/// differentiate between several instances of #TV_StringCallback.
/// \return
///    - #TV_OK always. Callback will be called once for each available module
///      receiving \c 1, the name of the module which corresponds to the name
///      that would be returned from module_get_name(), and \c context.
///      After all modules have been enumerated, the callback receives a \c 0
///      and an empty string, and no further callbacks will be received.
TV_Result tv_enumerate_available_modules(TV_StringCallback callback,
                                         TV_Context context);

/// Access the currently set user module load path.
/// \param[out] path The user defined path searched for modules.
/// \return TV_OK
TV_Result tv_user_module_load_path(TV_CharArray path);

/// Access the fixed system module load path.
/// \param[out] path The fixed system path searched for modules.
/// \return TV_OK
TV_Result tv_system_module_load_path(TV_CharArray path);

/// Start a new scene given a loaded module.
/// \param[in] id Id of a loaded module.
/// \param[out] scene Id of the scene if it was created successfully.
/// \return
///    - #TV_NOT_IMPLEMENTED Currently. (If the given module will be removed)
///    - #TV_INVALID_ID (If no module with id exists)
///    - #TV_OK (if the scene was started successfully)
///    - (else, an error during scene creation occured.)
TV_Result tv_scene_from_module(TV_Id id, TV_Scene* scene);

/// Add a module to an existing scene.
/// \param[in] scene Id of the scene.
/// \param[in] id Id of the module.
/// \return
///    - #TV_NOT_IMPLEMENTED id currently.
TV_Result tv_scene_add_module(TV_Scene scene, TV_Id module);

/// Remove a scene without affecting the associated modules.
/// \param[in] scene Id of the scene.
/// \return
///    - #TV_NOT_IMPLEMENTED id currently.
TV_Result tv_scene_remove(TV_Scene scene);

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
/// \deprecated This method is not activated in the Red-Brick interface since it
/// does not make sense there.  We can't activate callbacks per module with the
/// generated api.  This method will probably be removed to focus on the
/// Red-Brick.
TV_Result tv_set_callback(TV_Id id, TV_Callback callback);

/// Enable a default callback which will be called for each result of every
/// module.
/// \param[in] callback The default callback.
TV_Result tv_enable_default_callback(TV_Callback callback);

/// Get the result of the latest execution of a given module.
/// \param[in] id The module in question.
/// \param[out] result The latest result of the module, if any.
/// \todo What else?
/// \return TV_INVALID_ID if no such module exists.
/// \return TV_OK else.
TV_Result tv_get_result(TV_Id module, TV_ModuleResult* result);

/// Get a string representation of a result code.
/// \param[in] code one of the TV_* values.
/// \return The associated string value.
TV_String tv_result_string(TV_Result code);

#ifdef __cplusplus
}
#endif
