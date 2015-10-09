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

#include <map>

#include "tinkervision_defines.h"

namespace tv {

class TVStringMap {
private:
    const std::map<TV_Result, TV_String> string_map_{
        {TV_OK, "Ok"},
        // 500...
        {TV_NOT_IMPLEMENTED, "Error - Not implemented"},
        {TV_INTERNAL_ERROR, "Error - Unknown internal error"},
        {TV_NODE_ALLOCATION_FAILED,
         "Error - Allocation of a scene node failed"},
        {TV_NO_ACTIVE_MODULES, "Error - No active modules"},
        // 550...
        {TV_CAMERA_NOT_AVAILABLE, "Error - Camera not available"},
        {TV_CAMERA_SETTINGS_FAILED, "Error - Setting camera properties failed"},
        // 600...
        {TV_INVALID_ID, "Error - ID is invalid"},
        {TV_MODULE_INITIALIZATION_FAILED,
         "Error - Initialization of module failed"},
        {TV_MODULE_ERROR_SETTING_PARAMETER,
         "Error - Module parameterization failed"},
        {TV_MODULE_NO_SUCH_PARAMETER, "Error - No such parameter"},
        // 650...
        {TV_EXEC_THREAD_FAILURE, "Error - The main thread did not react"},
        {TV_THREAD_RUNNING, "Error - The main thread is already running"},
        // 700...
        {TV_MODULE_DLOPEN_FAILED, "Error - Could not open requested module"},
        {TV_MODULE_DLSYM_FAILED,
         "Error - Required function not defined in module"},
        // 750...
        {TV_RESULT_NOT_AVAILABLE,
         "Error - Result requested where none is provided"},
        {TV_GLOBAL_CALLBACK_ACTIVE,
         "Error - Result of incompatible type requested"}};

public:
    static constexpr TV_String UNKNOWN_CODE{"Unknown result code"};
    static constexpr TV_String INTERNAL_ERROR{"libtfcv: Internal error"};

    TV_String operator[](TV_Result code) const {
        auto result = TVStringMap::UNKNOWN_CODE;
        try {
            auto it = string_map_.find(code);
            if (it != string_map_.end()) {
                result = it->second;
            }
        } catch (...) {
            result = TVStringMap::INTERNAL_ERROR;
        }
        return result;
    }
};
}
