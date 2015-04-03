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

#include "tinkervision_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/* colormatch */

TFV_Result colormatch_start(TFV_Id feature_id, TFV_Byte min_hue,
                            TFV_Byte max_hue, TFV_CallbackColormatch callback,
                            TFV_Context opaque);

TFV_Result colormatch_restart(TFV_Id feature_id);

TFV_Result colormatch_stop(TFV_Id feature_id);

TFV_Result colormatch_get(TFV_Id feature_id, TFV_Byte* min_hue,
                          TFV_Byte* max_hue);

/* motiondetect */

TFV_Result motiondetect_start(TFV_Id feature_id,
                              TFV_CallbackMotiondetect callback,
                              TFV_Context opaque);

/* api */

TFV_Result camera_available(void);

/**
 * Starts a dummy module keeping the Api up and running even if no 'real' module
 * is active.  This can be used to block the camera or if the resolution has to
 * be known before any module is running.  Subsequent calls will not start
 * another dummy.  The Dummy, once started, can only be quit by calling quit().
 * \return TFV_Ok if the dummy module was started or already running
 */
TFV_Result start_idle(void);

/**
 * Request the resolution of the camera frames.  This can only be called once
 * the camera is active, so in particular, if the resolution needs to be known
 * before a module can be started, start_idle() must be called.
 * \return
 *  - TFV_Ok if width and height are valid
 *  - TFV_CAMERA_NOT_AVAILABLE else
 */
TFV_Result get_resolution(TFV_Size* width, TFV_Size* height);

/**
 * Pause the Api, deactivating but not disabling every module.  The camera will
 * be released and no further callbacks will be called, but on start(), the Api
 * will be found in the exact same state as left (assuming that the camera can
 * be acquired again).
 */
TFV_Result stop(void);

TFV_Result start(void);

/**
 * Stop all modules and shutdown the api.  This is generally not necessary if
 * the client application terminates in controlled ways.  However, the
 * application should implement a kill-signal handler which calls quit.  Else
 * there is no way to shutdown the Api correctly in case of the client being
 * killed.
 */
TFV_Result quit(void);

TFV_Result set_execution_latency(TFV_UInt milliseconds);

TFV_String result_string(TFV_Result code);

/* streamer */

TFV_Result streamer_stream(TFV_Id streamer_id);

/* record */

TFV_Result snapshot(void);

#ifdef __cplusplus
}
#endif
