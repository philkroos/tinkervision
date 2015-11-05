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
#include <algorithm>

#include "module_loader.hh"
#include "exceptions.hh"
#include "filesystem.hh"
#include "logger.hh"

tv::ModuleLoader::ModuleLoader(std::string const& system_lib_load_path,
                               std::string const& user_lib_load_path)
    : system_load_path_(system_lib_load_path),
      user_load_path_(user_lib_load_path),
      dirwatch_(&ModuleLoader::_watched_directory_changed_callback, this) {

    auto modules = std::vector<std::string>{};
    auto paths = std::vector<std::string>{};

    list_available_modules(paths, modules);

    Log("MODULE_LOADER", "Starting with ", paths.size(), " available modules");

    for (size_t i = 0; i < modules.size(); ++i) {
        assert(_add_available_module(paths[i], strip_extension(modules[i])));
        auto const a = availables_.back();
        Log("MODULE_LOADER", a.loadpath, ": ", a.libname);
        for (auto const& p : a.parameter) {
            Log("MODULE_LOADER", p);
        }
    }

    // monitor changes
    dirwatch_.add_watched_extension("so");
    dirwatch_.watch(system_load_path_);
    dirwatch_.watch(user_load_path_);
}

bool tv::ModuleLoader::set_user_load_path(std::string const& load_path) {
    if (not is_directory(load_path)) {
        LogError("MODULE_LOADER", "Load path is not a directory: ", load_path);
        return false;
    }

    if (on_change_callback and not dirwatch_.watch(load_path)) {
        LogError("MODULE_LOADER", "Unknown error for load path ", load_path);
        return false;
    }

    /// \todo Notify the listener about modules that are no longer available.
    dirwatch_.unwatch(user_load_path_);
    user_load_path_ = load_path;
    return true;
}

void tv::ModuleLoader::list_available_modules(
    std::vector<std::string>& paths, std::vector<std::string>& modules) const {

    auto filter = [](std::string const&, std::string ext,
                     bool is_file) { return is_file and ext == "so"; };

    /// \todo Check here if the found libraries actually contain valid
    /// vision-modules.
    list_directory_content(system_load_path_, modules, filter);
    for (size_t i = 0; i < modules.size(); ++i) {
        paths.push_back(system_load_path_);
    }
    list_directory_content(user_load_path_, modules, filter);
    for (size_t i = paths.size(); i < modules.size(); ++i) {
        paths.push_back(user_load_path_);
    }

    /// All available modules are listed by name, i.e. the same as the filename
    /// but without extension .so.
    std::transform(modules.begin(), modules.end(), modules.begin(),
                   // ? Can't resolve strip_extension if used directly??
                   [&](std::string const& s) { return strip_extension(s); });
}

bool tv::ModuleLoader::load_module_from_library(ModuleWrapper** target,
                                                std::string const& libname,
                                                TV_Int id) {
    auto lib = (LibraryHandle) nullptr;

    // prefer user modules
    lib = _load_module_from_library(target, user_load_path_, libname, id);
    if (lib != nullptr) {
        handles_[*target] = {libname, user_load_path_, lib};
    } else {
        lib = _load_module_from_library(target, system_load_path_, libname, id);
        if (lib != nullptr) {
            handles_[*target] = {libname, system_load_path_, lib};
        } else {
            return false;
        }
    }

    Log("MODULE_LOADER", handles_[*target].loadpath, " -> ", libname);
    return true;
}

bool tv::ModuleLoader::destroy_module(ModuleWrapper* module) {
    auto entry = handles_.find(module);
    if (entry == handles_.cend()) {  // bug if this happens
        error_ = TV_INTERNAL_ERROR;
        return false;
    }

    // remove the library from the list of open handles
    auto handle = entry->second.handle;
    handles_.erase(module);

    // Destroy the object
    delete module;

    // Close the library
    return _free_lib(handle);
}

void tv::ModuleLoader::destroy_all(void) {
    auto libs = std::vector<LibraryHandle>{};
    for (auto lib : handles_) {
        libs.push_back(lib.second.handle);
    }

    /// First, destroy all internally wrapped modules.
    handles_.clear();

    /// Only then release all libraries.
    for (auto lib : libs) {
        (void)_free_lib(lib);
    }
}

TV_Result tv::ModuleLoader::last_error(void) {
    auto last = error_;
    error_ = TV_OK;
    return last;
}

void tv::ModuleLoader::update_on_changes(std::function<
    void(std::string const& directory, std::string const& filename,
         Dirwatch::Event event)> callback) {

    Log("MODULE_LOADER", "Registering callback for directory changes");
    on_change_callback = callback;
}

bool tv::ModuleLoader::library_available(std::string const& libname) const {
    return availables_.cend() !=
           std::find_if(availables_.cbegin(), availables_.cend(),
                        [&](AvailableModule const& module) {
               return module.libname == libname;
           });
}

bool tv::ModuleLoader::library_parameter_count(std::string const& libname,
                                               size_t& count) const {
    return availables_.cend() !=
           std::find_if(availables_.cbegin(), availables_.cend(),
                        [&](AvailableModule const& module) {
               if (module.libname == libname) {
                   count = module.parameter.size();
                   return true;
               }
               return false;
           });
}

size_t tv::ModuleLoader::libraries_count(void) const {
    return availables_.size();
}

bool tv::ModuleLoader::library_name_and_path(size_t count, std::string& name,
                                             std::string& path) const {
    if (count >= libraries_count()) {
        return false;
    }
    name = availables_[count].libname;
    path = availables_[count].loadpath;
    return true;
}

bool tv::ModuleLoader::library_describe_parameter(
    std::string const& libname, size_t number, std::string& name,
    parameter_t& min, parameter_t& max, parameter_t& def) const {

    auto it = std::find_if(
        availables_.cbegin(), availables_.cend(),
        [&libname](AvailableModule const& m) { return m.libname == libname; });

    if (it == availables_.cend() or it->parameter.size() <= number) {
        return false;
    }

    name = it->parameter[number].name();
    min = it->parameter[number].min();
    max = it->parameter[number].max();
    def = it->parameter[number].get();
    return true;
}

bool tv::ModuleLoader::_free_lib(LibraryHandle handle) {
    (void)dlerror();
    auto result = dlclose(handle);

    if (result != 0) {
        Log("MODULE_LOADER", "dlclose(..): (", result, ") ", dlerror());
        error_ = TV_MODULE_DLCLOSE_FAILED;
        return false;
    }
    return true;
}

tv::ModuleLoader::LibraryHandle tv::ModuleLoader::_load_module_from_library(
    ModuleWrapper** target, std::string const& library_root,
    std::string const& libname, TV_Int id) {

    auto handle = dlopen((library_root + libname + ".so").c_str(), RTLD_LAZY);
    if (not handle) {
        LogWarning("MODULE_LOADER", "dlopen(", libname, "): ", dlerror());
        error_ = TV_MODULE_DLOPEN_FAILED;
        return nullptr;
    }

    // check for required functions
    for (auto const& func : required_functions_) {
        (void)dlerror();  // reset errortracker
        (void)dlsym(handle, func.c_str());
        auto error = dlerror();
        if (error) {
            LogWarning("API", "dlsym(.., ", func, "): ", error);
            error_ = TV_MODULE_DLSYM_FAILED;
            _free_lib(handle);
            return nullptr;
        }
    }

    try {
        // auto shared_object = ConstructorFunction(dlsym(handle, "create"))();
        *target = new ModuleWrapper(
            ModuleWrapper::Constructor(dlsym(handle, "create")),
            ModuleWrapper::Destructor(dlsym(handle, "destroy")), id,
            library_root);

    } catch (Exception& ce) {
        LogError("MODULE_LOADER", ce.what());
        return nullptr;
    }

    return handle;
}

void tv::ModuleLoader::_watched_directory_changed_callback(
    Dirwatch::Event event, std::string const& dir, std::string const& file) {
    Log("MODULE_LOADER", "Received change for ", file, " in ", dir);

    // Modify list of available events accordingly
    if (event == Dirwatch::Event::FILE_CREATED) {
        assert(_add_available_module(dir, strip_extension(file)));

    } else if (event == Dirwatch::Event::FILE_DELETED) {
        auto it = std::find_if(availables_.cbegin(), availables_.cend(),
                               [&](AvailableModule const& a) {
            return a.libname == strip_extension(file) and a.loadpath == dir;
        });

        if (it != availables_.cend()) {
            availables_.erase(it);
        }
    }

    // Notify user
    if (on_change_callback) {
        /// \todo Check here if the found library actually contains a valid
        /// vision-module.
        on_change_callback(dir, file, event);
    }
}

bool tv::ModuleLoader::_add_available_module(std::string const& path,
                                             std::string const& name) {
    auto ok = false;  // pessimistic
    auto tmp = (ModuleWrapper*)(nullptr);
    auto const id = 100;  // doesn't matter

    auto lib = _load_module_from_library(&tmp, path, name, id);

    if (not lib) {  // should not happen
        LogError("MODULE_LOADER", "Construction error");

    } else if (not tmp->initialize()) {  // must be initializable
        LogError("MODULE_LOADER", "Module initialization error.");

    } else {

        availables_.push_back({name, path, {}});
        tmp->get_parameters_list(availables_.back().parameter);
        ok = true;
    }

    if (tmp) {
        // Destroy the temporary
        delete tmp;
    }

    if (lib) {
        // Close the library
        _free_lib(lib);
    }
    return ok;
}
