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

#include <stdint.h>

typedef int TFV_Bool;
typedef uint8_t TFV_Byte;
typedef unsigned char TFV_ImageData;

typedef int8_t TFV_Short;
typedef int32_t TFV_Int;

typedef const char* TFV_String;
typedef unsigned TFV_UInt;
typedef TFV_Short TFV_Id;
typedef TFV_Int TFV_Result;
typedef size_t TFV_Size;
typedef void* TFV_Context;

typedef void (*TFV_CallbackColormatch)(TFV_Id, TFV_Int x, TFV_Int y,
                                       TFV_Context);

typedef void (*TFV_CallbackMotiondetect)(TFV_Id, TFV_Int x_topleft,
                                         TFV_Int y_topleft,
                                         TFV_Int x_bottomright,
                                         TFV_Int y_bottomright, TFV_Context);

/* result codes */

/* General 'ok' results */
#define TFV_OK 0
#define TFV_NEW_FEATURE_CONFIGURED 10
#define TFV_FEATURE_RECONFIGURED 11

/* General errors: 500... */
#define TFV_NOT_IMPLEMENTED 500
#define TFV_INTERNAL_ERROR 501
#define TFV_UNKNOWN_ERROR 502
#define TFV_INVALID_CONFIGURATION 503

/* Camera errors: 550... */
#define TFV_CAMERA_ACQUISITION_FAILED 550
#define TFV_CAMERA_NOT_AVAILABLE 551
#define TFV_CAMERA_SETTINGS_FAILED 552

/* Configuration errors: 600... */
#define TFV_UNCONFIGURED_ID 600
#define TFV_FEATURE_CONFIGURATION_FAILED 601
#define TFV_INVALID_ID 602
#define TFV_MODULE_INITIALIZATION_FAILED 603

/* System thread errors: 650... */
#define TFV_EXEC_THREAD_FAILURE 650
