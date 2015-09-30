/// \file module_loader.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Implementation of ModuleLoader.
///
/// This file is part of Tinkervision - Vision Library for Tinkerforge Redbrick
/// \sa https://github.com/Tinkerforge/red-brick
/// \sa module_loader.hh
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

#include <dlfcn.h>
#include <cassert>

#include "module_loader.hh"
#include "exceptions.hh"
#include "filesystem.hh"
#include "logger.hh"

void tfv::ModuleLoader::list_available_modules(
    std::vector<std::string>& modules) const {

    auto filter = [](std::string const&, std::string ext,
                     bool is_file) { return is_file and ext == "so"; };

    /// \todo Check here if the found libraries actually contain valid
    /// vision-modules.
    list_directory_content(system_load_path_, modules, filter);
    list_directory_content(user_load_path_, modules, filter);
}

bool tfv::ModuleLoader::load_module_from_library(Module** target,
                                                 std::string const& libname,
                                                 TFV_Int id) {
    // prefer user modules
    if (not(_load_module_from_library(target, user_load_path_, libname, id) or

            _load_module_from_library(target, system_load_path_, libname,
                                      id))) {

        LogError("MODULE_LOADER", "Loading of library ", libname, " failed.");
        return false;
    }

    return true;
}

bool tfv::ModuleLoader::destroy_module(Module* module) {
    auto entry = handles_.find(module);
    if (entry == handles_.cend()) {  // bug if this happens
        error_ = TFV_INTERNAL_ERROR;
        return false;
    }

    auto handle = entry->second.handle;
    handles_.erase(module);
    DestructorFunction(dlsym(handle, "destroy"))(module->executable());

    delete module;
    return _free_lib(handle);
}

void tfv::ModuleLoader::destroy_all(void) {
    for (auto lib : handles_) {
        auto handle = lib.second.handle;
        auto name = lib.second.libname;

        Log("MODULE_LOADER", "Close library ", name);
        if (0 != dlclose(handle)) {
            LogError("API::quit", "dlclose(", name, "): ", dlerror());
            // Can't do nothing about that.
        }
    }
    handles_.clear();
}

TFV_Result tfv::ModuleLoader::last_error(void) {
    auto last = error_;
    error_ = TFV_OK;
    return last;
}

bool tfv::ModuleLoader::_free_lib(LibraryHandle handle) {
    (void)dlerror();
    auto result = dlclose(handle);

    if (result != 0) {
        Log("MODULE_LOADER", "dlclose(..): (", result, ") ", dlerror());
        error_ = TFV_MODULE_DLCLOSE_FAILED;
        return false;
    }
    return true;
}

bool tfv::ModuleLoader::_load_module_from_library(
    Module** target, std::string const& library_root,
    std::string const& libname, TFV_Int id) {

    auto handle = dlopen((library_root + libname + ".so").c_str(), RTLD_LAZY);
    if (not handle) {
        LogWarning("MODULE_LOADER", "dlopen(", libname, "): ", dlerror());
        error_ = TFV_MODULE_DLOPEN_FAILED;
        return false;
    }

    // check for required functions
    for (auto const& func : required_functions_) {
        (void)dlerror();  // reset errortracker
        (void)dlsym(handle, func.c_str());
        auto error = dlerror();
        if (error) {
            LogWarning("API", "dlsym(.., ", func, "): ", error);
            error_ = TFV_MODULE_DLSYM_FAILED;
            _free_lib(handle);
            return false;
        }
    }

    try {
        auto shared_object = ConstructorFunction(dlsym(handle, "create"))();
        *target = new Module(shared_object, id);

    } catch (Exception& ce) {
        LogError("MODULE_LOADER", ce.what());
        return false;
    }

    Log("MODULE_LOADER", "Loaded ", libname, " from ", library_root);
    handles_[*target] = {libname, handle};
    return true;
}
