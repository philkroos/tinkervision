/// \file tinkervision_defines.h
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Definitions for the Tinkervision library.
///
/// This file is part of Tinkervision - Vision Library for Tinkerforge Redbrick
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

#ifndef TINKERVISION_DEFINES_H
#define TINKERVISION_DEFINES_H

#include <stdint.h>

///< Maximum length of strings in and out of Tinkervision (including '\0')
#define TV_STRING_SIZE 30

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
typedef void (*TV_LibrariesCallback)(char const* name, char const* path,
                                     int8_t status, void* context);

#define TV_UNUSED_ID -1

#define SYS_MODULES_PATH "/usr/lib/tinkervision/"
#define MODULES_FOLDER "lib"      ///< Relative to USER_PREFIX (compiler define)
#define DATA_FOLDER "data"        ///< Relative to USER_PREFIX (compiler define)
#define SCRIPTS_FOLDER "scripts"  ///< Relative to USER_PREFIX (compiler define)

/* result codes */

///< Default 'no-error' result
#define TV_OK 0

#ifndef DEFAULT_CALL
///< Check for results every grain ms.
#define DELAY_GRAIN 100
///< Timout occurs after this many #DELAY_GRAIN.
#define GRAINS 10
///< Special result: Timeout occured, result buffered.
#define TV_RESULT_BUFFERED 1
#endif

/* General errors: */
#define TV_NOT_IMPLEMENTED -1
#define TV_INTERNAL_ERROR -2
#define TV_INVALID_ARGUMENT -3
#define TV_BUSY -4

///< Could not allocate a node in a SceneTree
#define TV_NODE_ALLOCATION_FAILED -11
#define TV_NO_ACTIVE_MODULES -12

/* Camera errors: */
#define TV_CAMERA_NOT_AVAILABLE -21
#define TV_CAMERA_SETTINGS_FAILED -22

///< An id passed to Api is not registered as Module
#define TV_INVALID_ID -31
#define TV_MODULE_INITIALIZATION_FAILED -32
#define TV_MODULE_NO_SUCH_PARAMETER -33
#define TV_MODULE_ERROR_SETTING_PARAMETER -34

/* System thread errors: */
#define TV_EXEC_THREAD_FAILURE -41
#define TV_THREAD_RUNNING -42

/* External library errors: */
#define TV_MODULE_DLOPEN_FAILED -51
#define TV_MODULE_DLSYM_FAILED -52
#define TV_MODULE_DLCLOSE_FAILED -53
#define TV_MODULE_CONSTRUCTION_FAILED -54
#define TV_MODULE_NOT_AVAILABLE -55

/* Callback/Result request errors: */
#define TV_RESULT_NOT_AVAILABLE -61
#define TV_GLOBAL_CALLBACK_ACTIVE -62

#endif
