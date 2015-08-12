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

#include <dlfcn.h>
#include <cassert>

#include "module_loader.hh"
#include "logger.hh"

TFV_Result tfv::ModuleLoader::last_error(void) {
    auto last = error_;
    error_ = TFV_OK;
    return last;
}

bool tfv::ModuleLoader::load_module_from_library(Module** target,
                                                 std::string const& libname,
                                                 TFV_Int id, Module::Tag tags) {

    auto handle = dlopen((load_path_ + libname + ".so").c_str(), RTLD_LAZY);
    if (not handle) {
        LogError("MODULE_LOADER", "dlopen(", libname, "): ", dlerror());
        error_ = TFV_MODULE_DLOPEN_FAILED;
        return false;
    }

    // check for required functions
    for (auto const& func : required_functions_) {
        (void)dlerror();  // reset errortracker
        (void)dlsym(handle, func.c_str());
        auto error = dlerror();
        if (error) {
            Log("API", "dlsym(.., ", func, "): ", dlerror());
            error_ = TFV_MODULE_DLSYM_FAILED;
            _free_lib(handle);
            return false;
        }
    }

    *target = ConstructorFunction(dlsym(handle, "create"))(id, tags);
    assert(*target != nullptr);  // why would it?

    handles_[*target] = {libname, handle};
    return true;
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

bool tfv::ModuleLoader::destroy_module(Module* module) {
    auto entry = handles_.find(module);
    if (entry == handles_.cend()) {  // bug if this happens
        error_ = TFV_INTERNAL_ERROR;
        return false;
    }

    auto handle = entry->second.handle;
    handles_.erase(module);
    DestructorFunction(dlsym(handle, "destroy"))(module);

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
