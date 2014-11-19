/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014 philipp.kroos@fh-bielefeld.de

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
typedef void TFV_Void;
typedef uint8_t TFV_Byte;
typedef unsigned char TFV_ImageData;
typedef int TFV_Int;
typedef const char* TFV_String;
typedef TFV_Int TFV_Id;
typedef TFV_Int TFV_Result;
typedef void* TFV_Context;
typedef void (*TFV_Callback)(TFV_Id, TFV_Int*, TFV_Int*, TFV_Int*, TFV_Int*,
                             TFV_Byte, TFV_Context);
// TFV_RESULT feature_found (int feature_id, x[], y[], width[], height[], count,
// Context opaque);

/* configurable values */

// Each cam can be used for so much configurations
#define TFV_MAX_USERS_PER_CAM 5

/* result codes */

// General 'ok' results
#define TFV_OK 0
#define TFV_NEW_FEATURE_CONFIGURED 10
#define TFV_FEATURE_RECONFIGURED 11

// General errors: 500...
#define TFV_NOT_IMPLEMENTED 500
#define TFV_INTERNAL_ERROR 501
#define TFV_UNKNOWN_ERROR 502
#define TFV_INVALID_CONFIGURATION 503

// Camera errors: 550...
#define TFV_CAMERA_ACQUISITION_FAILED 550

// Configuration errors: 600...
#define TFV_UNCONFIGURED_ID 600
#define TFV_FEATURE_CONFIGURATION_FAILED 601
#define TFV_INVALID_ID 602
