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

/** \file tinkervision_defines.h
    Common definitions for the public and internal apis.
*/
#include <stdint.h>

/* \todo: Use these types everywhere, e.g. see cameracontrol. */
typedef int8_t TFV_Bool;
typedef int8_t TFV_Short;
typedef int16_t TFV_Word;
typedef int32_t TFV_Long;
typedef uint16_t TFV_UWord;
typedef uint32_t TFV_UInt;
typedef uint8_t TFV_Byte;
typedef const char* TFV_String;
#define TFV_CHAR_ARRAY_SIZE 20
typedef char TFV_CharArray[TFV_CHAR_ARRAY_SIZE];

typedef TFV_Word TFV_Int;

typedef TFV_Byte TFV_ImageData;
typedef TFV_Short TFV_Id;
typedef TFV_UWord TFV_Size;
typedef TFV_Int TFV_Scene;
typedef TFV_Int TFV_Result;
typedef void* TFV_Context;

typedef void (*TFV_CallbackValue)(TFV_Id, TFV_Size x, TFV_Context);
typedef void (*TFV_CallbackPoint)(TFV_Id, TFV_Size x, TFV_Size y, TFV_Context);
typedef void (*TFV_CallbackRectangle)(TFV_Id, TFV_Size x_topleft,
                                      TFV_Size y_topleft, TFV_Size width,
                                      TFV_Size height, TFV_Context);
typedef void (*TFV_CallbackString)(TFV_Id, TFV_String string, TFV_Context);

typedef TFV_CallbackPoint TFV_CallbackColormatch;
typedef TFV_CallbackRectangle TFV_CallbackMotiondetect;

#define TFV_UNUSED_ID -1

/* Library configuration */
#define SYS_MODULE_LOAD_PATH "/usr/local/lib/tinkervision/"
#define ADD_MODULE_LOAD_PATH "/tmp/lib/tinkervision/"
/* result codes */

/** Default 'no-error' result
 */
#define TFV_OK 0
#define TFV_NEW_FEATURE_CONFIGURED 10
#define TFV_FEATURE_RECONFIGURED 11

/* General errors: 500... */
#define TFV_NOT_IMPLEMENTED 500
#define TFV_INTERNAL_ERROR 501
#define TFV_UNKNOWN_ERROR 502
#define TFV_INVALID_CONFIGURATION 503
/** Could not allocate a node in a SceneTree */
#define TFV_NODE_ALLOCATION_FAILED 504
#define TFV_NO_ACTIVE_MODULES 505

/* Camera errors: 550... */
#define TFV_CAMERA_ACQUISITION_FAILED 550
#define TFV_CAMERA_NOT_AVAILABLE 551
#define TFV_CAMERA_SETTINGS_FAILED 552

/* Configuration errors: 600... */
#define TFV_UNCONFIGURED_ID 600
#define TFV_FEATURE_CONFIGURATION_FAILED 601
/** An id passed to Api is not registered as Module */
#define TFV_INVALID_ID 602
#define TFV_MODULE_INITIALIZATION_FAILED 603
#define TFV_MODULE_NO_SUCH_PARAMETER 604
#define TFV_MODULE_ERROR_SETTING_PARAMETER 605

/* System thread errors: 650... */
#define TFV_EXEC_THREAD_FAILURE 650
#define TFV_THREAD_RUNNING 651

/* External library errors: 700... */
#define TFV_MODULE_DLOPEN_FAILED 700
#define TFV_MODULE_DLSYM_FAILED 701
#define TFV_MODULE_DLCLOSE_FAILED 702

/* Callback/Result request errors: 750... */
#define TFV_RESULT_NOT_AVAILABLE 750
#define TFV_INCOMPATIBLE_RESULT_TYPE 751

/* Internally used where a return value has only temporary means. Never returned
 * outside: 1000... */
#define TFV_INTERNAL_NODE_UNCONFIGURED 1000
