/// \file strings.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Declares and defines Strings.
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

#include <map>

#include "tinkervision_defines.h"

namespace tv {

/// Container to map result codes of the api to strings.
class Strings {
private:
    const std::map<int16_t, char const*> string_map_{
        {TV_OK, "Ok"},
        // -11...
        {TV_NOT_IMPLEMENTED, "Not implemented"},
        {TV_INTERNAL_ERROR, "Unknown internal error"},
        {TV_INVALID_ARGUMENT, "Invalid argument passed"},
        {TV_NODE_ALLOCATION_FAILED, "Allocation of a scene node failed"},
        {TV_NO_ACTIVE_MODULES, "No active modules"},
        // -21...
        {TV_CAMERA_NOT_AVAILABLE, "Camera not available"},
        {TV_CAMERA_SETTINGS_FAILED, "Setting camera properties failed"},
        // -31...
        {TV_INVALID_ID, "ID is invalid"},
        {TV_MODULE_INITIALIZATION_FAILED, "Initialization of module failed"},
        {TV_MODULE_ERROR_SETTING_PARAMETER, "Module parameterization failed"},
        {TV_MODULE_NO_SUCH_PARAMETER, "No such parameter"},
        // -41...
        {TV_EXEC_THREAD_FAILURE, "The main thread did not react"},
        {TV_THREAD_RUNNING, "The main thread is already running"},
        // -51...
        {TV_MODULE_DLOPEN_FAILED, "Could not open requested module"},
        {TV_MODULE_DLSYM_FAILED, "Required function not defined in module"},
        {TV_MODULE_DLCLOSE_FAILED, "Closing the library failed"},
        {TV_MODULE_CONSTRUCTION_FAILED, "Library construction error"},
        {TV_MODULE_NOT_AVAILABLE, "Library is not available"},
        // -61...
        {TV_RESULT_NOT_AVAILABLE, "Result requested where none is provided"},
        {TV_GLOBAL_CALLBACK_ACTIVE, "Result of incompatible type requested"}};

public:
    static constexpr char const* UNKNOWN_CODE{"Unknown result code"};
    static constexpr char const* INTERNAL_ERROR{"tinkervision: Internal error"};

    char const* operator[](int16_t code) const {
        auto result = Strings::UNKNOWN_CODE;
        try {
            auto it = string_map_.find(code);
            if (it != string_map_.end()) {
                result = it->second;
            }
        } catch (...) {
            result = Strings::INTERNAL_ERROR;
        }
        return result;
    }
};
}
