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

#ifndef TINKERVISION_DEFINES_H
#define TINKERVISION_DEFINES_H

/** \file tinkervision_defines.h
    Common definitions for the public and internal apis.
*/
#include <stdint.h>

///< Maximum length of strings in and out of Tinkervision.
#define TV_STRING_SIZE 24

typedef struct TV_ModuleResult {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    char string[TV_STRING_SIZE];
} TV_ModuleResult;

/// General callback applicable for every module that produces a result.
typedef void (*TV_Callback)(int8_t, TV_ModuleResult result, void*);
typedef void (*TV_StringCallback)(int8_t, char const* string, void* context);
/// \todo Too much data!! Status must be int.
typedef void (*TV_LibrariesCallback)(char const* name, char const* path,
                                     char const* status, void* context);

#define TV_UNUSED_ID -1

/// Systemwide vision module load path.
#define SYS_MODULE_LOAD_PATH "/usr/lib/tinkervision/"
/// User accessible vision module load path.
#define ADD_MODULE_LOAD_PATH "/tmp/lib/tinkervision/"
/* result codes */

/** Default 'no-error' result
 */
#define TV_OK 0

/* General errors: 500... */
#define TV_NOT_IMPLEMENTED 500
#define TV_INTERNAL_ERROR 501
#define TV_INVALID_ARGUMENT 502

/** Could not allocate a node in a SceneTree */
#define TV_NODE_ALLOCATION_FAILED 503
#define TV_NO_ACTIVE_MODULES 504

/* Camera errors: 550... */
#define TV_CAMERA_NOT_AVAILABLE 551
#define TV_CAMERA_SETTINGS_FAILED 552

/** An id passed to Api is not registered as Module */
#define TV_INVALID_ID 600
#define TV_MODULE_INITIALIZATION_FAILED 601
#define TV_MODULE_NO_SUCH_PARAMETER 602
#define TV_MODULE_ERROR_SETTING_PARAMETER 603

/* System thread errors: 650... */
#define TV_EXEC_THREAD_FAILURE 650
#define TV_THREAD_RUNNING 651

/* External library errors: 700... */
#define TV_MODULE_DLOPEN_FAILED 700
#define TV_MODULE_DLSYM_FAILED 701
#define TV_MODULE_DLCLOSE_FAILED 702

/* Callback/Result request errors: 750... */
#define TV_RESULT_NOT_AVAILABLE 750
#define TV_GLOBAL_CALLBACK_ACTIVE 751

/* Internally used where a return value has only temporary means. Never returned
 * outside: 1000... */
#define TV_INTERNAL_NODE_UNCONFIGURED 1000

#endif /* TINKERVISION_DEFINES_H */
