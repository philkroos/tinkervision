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
///    - TFV_CAMERA_NOT_AVAILABLE if not.
///    - TFV_OK else.
TFV_Result camera_available(void);

/// Selects a framesize WxH.
/// This can only be done while no modules are active.
/// This does not guarantee that the requested framesize will be used. It is
/// only
/// checked once the camera device is opened if the settings are supported, e.g.
/// during module_start().
/// \param[in] width
/// \param[in] height
/// \return
///   - #TFV_OK if no module was running.
///   - #TFV_CAMERA_SETTINGS_FAILED if the settings are ignored.
TFV_Result preselect_framesize(TFV_Size width, TFV_Size height);

/// Starts a dummy module keeping the Api up and running even if no 'real'
/// module
/// is active.  This can be used to block the camera or if the resolution has to
/// be known before any module is running.  Subsequent calls will not start
/// another dummy.  The Dummy, once started, can only be quit by calling quit().
/// \return
///   - #TFV_OK if the dummy module was started or already running.
///   - An error code if the module failed to load.
TFV_Result start_idle(void);

/// Introduce a delay into the execution to save processing power.  All
/// vision-modules registered and started in the api will be executed
/// sequentially during one execution loop. The execution latency set here is
/// the
/// minimum delay between two loops, i.e. a sort of minimum inverse framerate
/// (frames are grabbed once at the beginning of each loop).
/// \param[in] milliseconds The minimum delay between the beginning of two
/// execution loops.
/// \return TFV_OK in any case. There is however a minimum internal latency of
/// currently 20ms which won't be ignored.
TFV_Result set_execution_latency(TFV_UInt milliseconds);

/// Request the resolution of the camera frames.  This can only be called once
/// the camera is active, so in particular, if the resolution needs to be known
/// before a module can be started, start_idle() must be called.
/// \param[out] width
/// \param[out] height
/// \return
///  - #TFV_Ok if width and height are valid
///  - #TFV_CAMERA_NOT_AVAILABLE else
TFV_Result get_resolution(TFV_Size* width, TFV_Size* height);

/// Pause the Api, deactivating but not disabling every module.  The camera will
/// be released and no further callbacks will be called, but on start(), the Api
/// will be found in the exact same state as left (assuming that the camera can
/// be acquired again).
///     - #TFV_OK on success.
///     - #TFV_EXEC_THREAD_FAILURE on error.
TFV_Result stop(void);

/// Restart the API from paused state, initiated through call to stop().
/// \return
///   - #TFV_OK if the api was stoppend and is running now.
///   - an error code else; also if the api was already running.
TFV_Result start(void);

/// Stop all modules and shutdown the api.  This is generally not necessary if
/// the client application terminates in controlled ways.  However, the
/// application should implement a kill-signal handler which calls quit.  Else
/// there is no way to shutdown the Api correctly in case of the client being
/// killed.
/// \return TFV_OK
TFV_Result quit(void);

/// Parameterize a module.
/// \param[in] module_id Id of the module to be parameterized.
/// \param[in] parameter name of the parameter to be set.
/// \param[in] value Value to be set for parameter.
/// \return
///   - #TFV_MODULE_NO_SUCH_PARAMETER if the module does not support parameter.
///   - #TFV_MODULE_ERROR_SETTING_PARAMETER a module internal error during
/// setting
/// of the parameter.
///   - #TFV_INVALID_ID if no module exists with module_id
///   - #TFV_OK else.
TFV_Result set_parameter(TFV_Id module_id, TFV_String const parameter,
                         TFV_Int value);

/// Return the current value of a modules parameter.
/// \param[in] module_id The id of the module in question.
/// \param[in] parameter Name of the parameter in question.
/// \param[out] value Current value of parameter.
/// \return
///   - #TFV_INVALID_ID if no module exists with module_id.
///   - #TFV_MODULE_NO_SUCH_PARAMETER if the module does not support parameter.
///   - #TFV_OK else.
TFV_Result get_parameter(TFV_Id module_id, TFV_String const parameter,
                         TFV_Int* value);

/// Start a vision module identified by its library name.
/// The requested module will be loaded and started if it is found in one of the
/// available library search paths #SYS_MODULE_LOAD_PATH or
/// #ADD_MODULE_LOAD_PATH.  On success, it will be assigned a unique id.
/// \param[in] name The name of the requested module, which is the exact
/// filename of the  corresponding library without extension.
/// \param[out] id On success, the loaded module will be accessible with this
/// id.
/// \return
///    - #TFV_INTERNAL_ERROR if an id clash occured. This is an open but
///      unlikely bug.
///    - #TFV_CAMERA_NOT_AVAILABLE if the camera could not be opened.
///    - #TFV_MODULE_INITIALIZATION_FAILED The module could not be loaded. Maybe
///      an invalid name passed.
///    - #TFV_OK fine, module loaded and active.
TFV_Result module_start(TFV_String name, TFV_Id* id);

/// Disable a module without removing it.
/// A disabled module won't be executed, but it is still available for
/// configuration or reactivation. The associated camera will be released if it
/// is not used by other modules.
/// \see module_restart()
/// \param[in] id Id of the module to be stopped.
/// \return
///    - #TFV_INVALID_ID if no such module exists.
///    - #TFV_OK else, the module is inactive.
TFV_Result module_stop(TFV_Id id);

/// Restart a module that has been stopped with module_stop().
/// \param[in] id Id of the module to be restarted.
/// \return
///    - #TFV_INVALID_ID if no module is registered with
///      the given id.
///    - #TFV_CAMERA_ACQUISATION_FAILED if the camera is not available
///    - #TFV_OK iff the module is running.
TFV_Result module_restart(TFV_Id id);

/// Deactivate and remove a module.
/// The id of a removed module is invalid afterwards.
/// \param[in] id Id of the module to be restarted.
/// \return
///    - #TFV_NOT_IMPLEMENTED if scenes are active.
///    - #TFV_INVALID_ID if the module does not exist.
///    - #TFV_OK if removal succeeded.
TFV_Result module_remove(TFV_Id id);

/// Get the name of a loaded module.
/// \param[in] id The id of the module.
/// \param[out] name The name, which is exactly the name that would be passed to
/// module_start().
/// \return
///    - #TFV_INVALID_ID If no such module exists.
///    - #TFV_OK else, name will be valid.
TFV_Result module_get_name(TFV_Id id, TFV_CharArray name);

/// Get the identifiers of all parameters of a loaded module.
/// \param[in] id The id of the module.
/// \param[in] callback The given method will be called for each parameter's
/// name.
/// \param[in] context A pointer to something, which can be used to
/// differentiate between several instances of #TFV_StringCallback.
/// \return
///    - #TFV_INVALID_ID If no such module exists.
///    - #TFV_OK else, callback will be called with the id of the module, one
///      parameter name per time, and \c context. After all parameter names have
///      been passed, the callback receives an id of \c 0 and no further
///      callbacks will be received.
TFV_Result module_enumerate_parameters(TFV_Id module_id,
                                       TFV_StringCallback callback,
                                       TFV_Context context);

/// Get the names of all available modules.
/// \param[in] callback The given method will be called for each loadable
/// module.
/// \param[in] context A pointer to something, which can be used to
/// differentiate between several instances of #TFV_StringCallback.
/// \return
///    - #TFV_OK always. Callback will be called once for each available module
///      receiving \c 1, the name of the module which corresponds to the name
///      that would be returned from module_get_name(), and \c context.
///      After all modules have been enumerated, the callback receives a \c 0
///      and an empty string, and no further callbacks will be received.
TFV_Result enumerate_available_modules(TFV_StringCallback callback,
                                       TFV_Context context);

/// Start a new scene given a loaded module.
/// \param[in] id Id of a loaded module.
/// \param[out] scene Id of the scene if it was created successfully.
/// \return
///    - #TFV_NOT_IMPLEMENTED Currently. (If the given module will be removed)
///    - #TFV_INVALID_ID (If no module with id exists)
///    - #TFV_OK (if the scene was started successfully)
///    - (else, an error during scene creation occured.)
TFV_Result scene_from_module(TFV_Id id, TFV_Scene* scene);

/// Add a module to an existing scene.
/// \param[in] scene Id of the scene.
/// \param[in] id Id of the module.
/// \return
///    - #TFV_NOT_IMPLEMENTED id currently.
TFV_Result scene_add_module(TFV_Scene scene, TFV_Id module);

/// Remove a scene without affecting the associated modules.
/// \param[in] scene Id of the scene.
/// \return
///    - #TFV_NOT_IMPLEMENTED id currently.
TFV_Result scene_remove(TFV_Scene scene);

/// Set a callback to the result of a specific module.
/// The given callback will be called after each execution of the specified
/// module, provided it has produced a result.
/// \param[in] id The module.
/// \param[in] callback The function to be called for each result.
/// \return
///    - #TFV_INVALID_ID if no such module exists.
///    - #TFV_GLOBAL_CALLBACK_ACTIVE if a default callback has been set
///       previously with enable_default_callback()
///    - #TFV_INTERNAL_ERROR That's a bug, please report.
///    - #TFV_OK if the callback has been set successfully.
/// \deprecated This method is not activated in the Red-Brick interface since it
/// does not make sense there.  We can't activate callbacks per module with the
/// generated api.  This method will probably be removed to focus on the
/// Red-Brick.
TFV_Result set_callback(TFV_Id id, TFV_Callback callback);

/// Enable a default callback which will be called for each result of every
/// module.
/// \param[in] callback The default callback.
TFV_Result enable_default_callback(TFV_Callback callback);

/// Get the result of the latest execution of a given module.
/// \param[in] id The module in question.
/// \param[out] result The latest result of the module, if any.
/// \todo What else?
/// \return TFV_INVALID_ID if no such module exists.
/// \return TFV_OK else.
TFV_Result get_result(TFV_Id module, TFV_ModuleResult* result);

/// Get a string representation of a result code.
/// \param[in] code one of the TFV_* values.
/// \return The associated string value.
TFV_String result_string(TFV_Result code);

#ifdef __cplusplus
}
#endif
