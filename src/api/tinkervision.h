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

// colormatch

TFV_Result colormatch_start(TFV_Id feature_id, TFV_Byte min_hue,
                            TFV_Byte max_hue, TFV_CallbackColormatch callback,
                            TFV_Context opaque);

TFV_Result colormatch_restart(TFV_Id feature_id);

TFV_Result colormatch_stop(TFV_Id feature_id);

TFV_Result colormatch_get(TFV_Id feature_id, TFV_Byte* min_hue,
                          TFV_Byte* max_hue);

TFV_Result camera_available(void);

// motiondetect

TFV_Result motiondetect_start(TFV_Id feature_id,
                              TFV_CallbackMotiondetect callback,
                              TFV_Context opaque);

// api

TFV_Result stop(void);

TFV_Result start(void);

TFV_Result quit(void);

TFV_Result set_execution_latency(TFV_UInt milliseconds);

TFV_String result_string(TFV_Result code);

// streamer

TFV_Result streamer_stream(TFV_Id streamer_id);

// record

TFV_Result snapshot(void);

#ifdef __cplusplus
}
#endif
