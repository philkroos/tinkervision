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

namespace tfv {

class TFVStringMap {
private:
    const std::map<TFV_Result, TFV_String> string_map_{
        {TFV_OK, "Ok"},
        {TFV_NEW_FEATURE_CONFIGURED, "New feature configured"},
        {TFV_FEATURE_RECONFIGURED, "Feature reconfigured"},
        // 500...
        {TFV_NOT_IMPLEMENTED, "Error - Not implemented"},
        {TFV_UNKNOWN_ERROR, "Error - Unknown error"},
        {TFV_INTERNAL_ERROR, "Error - Unknown internal error"},
        {TFV_INVALID_CONFIGURATION, "Error - Invalid feature configuration"},
        // 550...
        {TFV_CAMERA_ACQUISITION_FAILED, "Error - Camera not available"},
        // 600...
        {TFV_UNCONFIGURED_ID, "Error - Feature-Id not configured"},
        {TFV_FEATURE_CONFIGURATION_FAILED,
         "Error - Configuration of feature failed"},
        {TFV_INVALID_ID, "Error - ID is invalid"},
        // 650..
        {TFV_EXEC_THREAD_FAILURE, "Error - The main thread did not react"}};

public:
    static constexpr TFV_String UNKNOWN_CODE{"Unknown result code"};
    static constexpr TFV_String INTERNAL_ERROR{"libtfcv: Internal error"};

    TFV_String operator[](TFV_Result code) const {
        auto result = TFVStringMap::UNKNOWN_CODE;
        try {
            auto it = string_map_.find(code);
            if (it != string_map_.end()) {
                result = it->second;
            }
        } catch (...) {
            result = TFVStringMap::INTERNAL_ERROR;
        }
        return result;
    }
};
};
