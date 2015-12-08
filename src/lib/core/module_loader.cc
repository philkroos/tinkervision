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

tv::ModuleLoader::ModuleLoader(Environment const& environment)
    : environment_(environment),
      dirwatch_(&ModuleLoader::_watched_directory_changed_callback, this) {

    auto modules = std::vector<std::string>{};
    auto paths = std::vector<std::string>{};

    // list of .so files. Checked for validity in _add_available_module
    _possibly_available_modules(paths, modules);

    Log("MODULE_LOADER", "User libs dir: ", environment_.user_modules_path());
    Log("MODULE_LOADER", "Starting with ", paths.size(), " available modules");

    for (size_t i = 0; i < modules.size(); ++i) {
        if (not _add_available_module(paths[i], strip_extension(modules[i]))) {
            continue;
        }
        auto const& a = availables_.back();
        Log("MODULE_LOADER", a.loadpath, ": ", a.libname);
        for (auto const& p : a.parameter) {
            Log("MODULE_LOADER", p);
        }
    }

    // monitor changes
    dirwatch_.add_watched_extension("so");
    dirwatch_.watch(environment_.system_modules_path());
    dirwatch_.watch(environment_.user_modules_path());
}

tv::ModuleLoader::~ModuleLoader(void) {
    destroy_all();
    for (auto& a : availables_) {
        for (auto& p : a.parameter) {
            delete p;
        }
    }
}

bool tv::ModuleLoader::switch_user_load_path(std::string const& old_path,
                                             std::string const& load_path) {
    if (load_path == environment_.user_modules_path()) {
        LogWarning("MODULE_LOADER", "Load path does not change: ", load_path);
        return true;
    }

    if (not dirwatch_.watch(load_path)) {
        LogError("MODULE_LOADER", "Unknown error for load path ", load_path);
        return false;
    }

    std::vector<int> lost_modules;
    for (size_t i = 0; i < availables_.size(); ++i) {
        auto const& module = availables_[i];
        if (module.loadpath == old_path) {
            lost_modules.push_back(i);
            /// Notifies the listener about modules that are no longer
            /// available.
            if (on_change_callback) {
                on_change_callback(module.loadpath, module.libname,
                                   Dirwatch::Event::FILE_DELETED);
            }
        }
    }
    for (auto const& idx : lost_modules) {
        availables_.erase(availables_.cbegin() + idx);
    }

    dirwatch_.unwatch(old_path);
    dirwatch_.watch(environment_.user_modules_path());
    return true;
}

void tv::ModuleLoader::list_available_modules(
    std::vector<std::string>& paths, std::vector<std::string>& modules) const {

    /// All available modules are listed by name, i.e. the same as the filename
    /// but without extension .so.
    for (auto const& library : availables_) {
        paths.push_back(library.loadpath);
        modules.push_back(library.libname);
    }
}

void tv::ModuleLoader::_possibly_available_modules(
    std::vector<std::string>& paths, std::vector<std::string>& modules) const {

    auto filter = [](std::string const&, std::string ext,
                     bool is_file) { return is_file and ext == "so"; };

    list_directory_content(environment_.system_modules_path(), modules, filter);
    for (size_t i = 0; i < modules.size(); ++i) {
        paths.push_back(environment_.system_modules_path());
    }
    list_directory_content(environment_.user_modules_path(), modules, filter);
    for (size_t i = paths.size(); i < modules.size(); ++i) {
        paths.push_back(environment_.user_modules_path());
    }

    /// All available modules are listed by name, i.e. the same as the filename
    /// but without extension .so.
    std::transform(modules.begin(), modules.end(), modules.begin(),
                   // ? Can't resolve strip_extension if used directly??
                   [&](std::string const& s) { return strip_extension(s); });
}

bool tv::ModuleLoader::load_module_from_library(ModuleWrapper** target,
                                                std::string const& libname,
                                                int16_t id) {
    auto lib = (LibraryHandle) nullptr;

    if (not library_available(libname)) {

        error_ = TV_MODULE_NOT_AVAILABLE;
        LogError("MODULE_LOADER", libname, " not available.");
        return false;
    }

    // prefer user modules
    lib = _load_module_from_library(target, environment_.user_modules_path(),
                                    libname, id);
    if (lib != nullptr) {
        handles_[*target] = {libname, environment_.user_modules_path(), lib};
    } else {
        lib = _load_module_from_library(
            target, environment_.system_modules_path(), libname, id);
        if (lib != nullptr) {
            handles_[*target] = {libname, environment_.system_modules_path(),
                                 lib};
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
    for (auto const& lib : handles_) {
        libs.push_back(lib.second.handle);
    }

    /// First, destroy all internally wrapped modules.
    handles_.clear();

    /// Only then release all libraries.
    for (auto lib : libs) {
        (void)_free_lib(lib);
    }
}

int16_t tv::ModuleLoader::last_error(void) {
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
                                               uint16_t& count) const {
    return availables_.cend() !=
           std::find_if(availables_.cbegin(), availables_.cend(),
                        [&](AvailableModule const& module) {
               if (module.libname == libname) {
  		   count = static_cast<uint16_t>(module.parameter.size());
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

bool tv::ModuleLoader::library_get_parameter(std::string const& libname,
                                             size_t number,
                                             Parameter const** p) const {

    auto it = std::find_if(
        availables_.cbegin(), availables_.cend(),
        [&libname](AvailableModule const& m) { return m.libname == libname; });

    if (it == availables_.cend() or it->parameter.size() <= number) {
        return false;
    }

    *p = it->parameter[number];
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
    std::string const& libname, int16_t id) {

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
        *target = new ModuleWrapper(
            ModuleWrapper::Constructor(dlsym(handle, "create")),
            ModuleWrapper::Destructor(dlsym(handle, "destroy")), id,
            environment_, library_root);

    } catch (Exception& ce) {
        LogError("MODULE_LOADER", ce.what());
        error_ = TV_MODULE_CONSTRUCTION_FAILED;
        _free_lib(handle);
        *target = nullptr;
        return nullptr;
    }

    return handle;
}

void tv::ModuleLoader::_watched_directory_changed_callback(
    Dirwatch::Event event, std::string const& dir, std::string const& file) {
    Log("MODULE_LOADER", "Received change for ", file, " in ", dir);

    // Modify list of available events accordingly
    if (event == Dirwatch::Event::FILE_CREATED) {
        if (not _add_available_module(dir, strip_extension(file))) {
            Log("MODULE_LOADER", "Ignoring new library ", file, " in ", dir);
            return;
        }

    } else if (event == Dirwatch::Event::FILE_DELETED) {
        auto it = std::find_if(availables_.cbegin(), availables_.cend(),
                               [&](AvailableModule const& a) {
            return a.libname == strip_extension(file) and a.loadpath == dir;
        });

        if (it != availables_.cend()) {
            availables_.erase(it);
        } else {
            Log("MODULE_LOADER", "Ignoring deletion of ", file, " in ", dir);
            return;
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

    } else if (tmp->name() != name) {  // must be identifiable
        LogError("MODULE_LOADER", "Module name must equal file: ", tmp->name(),
                 "-", name);

    } else if (not tmp->initialize()) {  // must be initializable
        LogError("MODULE_LOADER", "Module initialization error.");

    } else {  // ok

        Log("MODULE_LOADER", "Adding module ", name, " from ", path);
        availables_.push_back({name, path, {}});

        // Get a copy of the supported parameters, to have them available even
        // if the module is not loaded.
        auto parameters = tmp->get_parameter_count();
        for (size_t i = 0; i < parameters; ++i) {
            auto const& p = tmp->get_parameter_by_number(i);

            if (p.type() == Parameter::Type::String) {
                std::string value;
                (void)p.get(value);
                availables_.back().parameter.push_back(
                    new StringParameter(p.name(), value, nullptr));
            } else if (p.type() == Parameter::Type::Numerical) {
                int32_t value;
                (void)p.get(value);
                availables_.back().parameter.push_back(
                    new NumericalParameter(p.name(), p.min(), p.max(), value));
            }
        }

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
