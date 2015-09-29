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

/** \file tinkervision.h

    Public portable interface to the Tinkervision library.

    This file provides a thin C-wrapper around the public api provided by
    tfv::Api.
*/

#include "tinkervision_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Checks if the camera device is available.
 */
TFV_Result camera_available(void);

/** Selects a framesize WxH.
 * This can only be done while no modules are active.
 * This does not guarantee that the requested framesize will be used. It is only
 * checked once the camera device is opened if the settings are supported, e.g.
 * during module_start().
 * \param[in] width
 * \param[in] height
 * \return
 *   - #TFV_OK if no module was running.
 *   - #TFV_CAMERA_SETTINGS_FAILED if the settings are ignored.
 */
TFV_Result preselect_framesize(TFV_Size width, TFV_Size height);

/**
 * Starts a dummy module keeping the Api up and running even if no 'real' module
 * is active.  This can be used to block the camera or if the resolution has to
 * be known before any module is running.  Subsequent calls will not start
 * another dummy.  The Dummy, once started, can only be quit by calling quit().
 * \return
 *   - #TFV_OK if the dummy module was started or already running.
 *   - An error code if the module failed to load.
 */
TFV_Result start_idle(void);

/**
 * Introduce a delay into the execution to save processing power.  All
 * vision-modules registered and started in the api will be executed
 * sequentially during one execution loop. The execution latency set here is the
 * minimum delay between two loops, i.e. a sort of minimum inverse framerate
 * (frames are grabbed once at the beginning of each loop).
 * \param[in] milliseconds The minimum delay between the beginning of two
 * execution loops.
 * \return TFV_OK in any case. There is however a minimum internal latency of
 * currently 20ms which won't be ignored.
 */
TFV_Result set_execution_latency(TFV_UInt milliseconds);

/**
 * Request the resolution of the camera frames.  This can only be called once
 * the camera is active, so in particular, if the resolution needs to be known
 * before a module can be started, start_idle() must be called.
 * \param[out] width
 * \param[out] height
 * \return
 *  - #TFV_Ok if width and height are valid
 *  - #TFV_CAMERA_NOT_AVAILABLE else
 */
TFV_Result get_resolution(TFV_Size* width, TFV_Size* height);

/**
 * Pause the Api, deactivating but not disabling every module.  The camera will
 * be released and no further callbacks will be called, but on start(), the Api
 * will be found in the exact same state as left (assuming that the camera can
 * be acquired again).
 *     - #TFV_OK on success.
 *     - #TFV_EXEC_THREAD_FAILURE on error.
 */
TFV_Result stop(void);

/**
 * Restart the API from paused state, initiated through call to stop().
 * \return
 *   - #TFV_OK if the api was stoppend and is running now.
 *   - an error code else; also if the api was already running.
 */
TFV_Result start(void);

/**
 * Stop all modules and shutdown the api.  This is generally not necessary if
 * the client application terminates in controlled ways.  However, the
 * application should implement a kill-signal handler which calls quit.  Else
 * there is no way to shutdown the Api correctly in case of the client being
 * killed.
 * \return TFV_OK
 */
TFV_Result quit(void);

/**
 * Parameterize a module.
 * \param[in] module_id Id of the module to be parameterized.
 * \param[in] parameter name of the parameter to be set.
 * \param[in] value Value to be set for parameter.
 * \return
 *   - #TFV_MODULE_NO_SUCH_PARAMETER if the module does not support parameter.
 *   - #TFV_MODULE_ERROR_SETTING_PARAMETER a module internal error during
 * setting
 * of the parameter.
 *   - #TFV_INVALID_ID if no module exists with module_id
 *   - #TFV_OK else.
 */
TFV_Result set_parameter(TFV_Id module_id, TFV_String const parameter,
                         TFV_Int value);

/**
 * Return the current value of a modules parameter.
 * \param[in] module_id The id of the module in question.
 * \param[in] parameter Name of the parameter in question.
 * \param[out] value Current value of parameter.
 * \return
 *   - #TFV_INVALID_ID if no module exists with module_id.
 *   - #TFV_MODULE_NO_SUCH_PARAMETER if the module does not support parameter.
 *   - #TFV_OK else.
 */
TFV_Result get_parameter(TFV_Id module_id, TFV_String const parameter,
                         TFV_Int* value);

TFV_Result module_start(TFV_String name, TFV_Id id);

TFV_Result module_stop(TFV_Id id);

TFV_Result module_restart(TFV_Id id);

TFV_Result module_remove(TFV_Id id);

TFV_Result scene_from_module(TFV_Id module, TFV_Scene* scene);
TFV_Result scene_add_module(TFV_Scene scene, TFV_Id module);
TFV_Result scene_remove(TFV_Scene scene);

TFV_Result set_value_callback(TFV_Id module, TFV_CallbackValue callback);
TFV_Result set_point_callback(TFV_Id module, TFV_CallbackPoint callback);
TFV_Result set_rect_callback(TFV_Id module, TFV_CallbackRectangle callback);
TFV_Result set_string_callback(TFV_Id module, TFV_CallbackString callback);

TFV_Result get_value_result(TFV_Id module, TFV_Size* value);
TFV_Result get_point_result(TFV_Id module, TFV_Size* x, TFV_Size* y);
TFV_Result get_rect_result(TFV_Id module, TFV_Size* x, TFV_Size* y,
                           TFV_Size* width, TFV_Size* height);
TFV_Result get_string_result(TFV_Id module, TFV_CharArray result);

TFV_String result_string(TFV_Result code);

#ifdef __cplusplus
}
#endif
