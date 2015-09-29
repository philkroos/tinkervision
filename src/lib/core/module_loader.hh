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

/** \class ModuleLoader
    \brief Load external modules, manage and release their so-handles.

    This class provides a wrapper around the shared object loading
    code.  Loaded shared objects are managed in an internal map
    indexed by the address of the actual modules loaded from the
    libraries.  As a consequence, a library is dlopen'ed once per
    module, even if it is already opened, and also dlclose'd once.

    \see The manpage for dlopen/dlsym/dlclose.
    \see SharedResource which is used to lock usage of loaded modules.
    \see Api::module_load, from where the instantiated library object
    is passed into SharedResource.
*/

#include <unordered_map>
#include <vector>
#include <dlfcn.h>

#include "module.hh"

namespace tfv {

class ModuleLoader {
public:
    using ConstructorFunction = TVModule* (*)();
    using DestructorFunction = void (*)(TVModule*);

    /**
     * c'tor initializing this class with a path where libraries are to be
     * found.
     */
    explicit ModuleLoader(std::string const& system_lib_load_path,
                          std::string const& user_lib_load_path)
        : system_load_path_(system_lib_load_path),
          user_load_path_(user_lib_load_path) {}

    /**
     * Load a vision-module from library libname.
     *
     * \param[inout] target Pointer to the Module pointer which will
     *               point to the loaded module.
     * \param[in] libname basename of the library to be opened.
     * \param[in] id First argument to the constructor function of the library.
     * \param[in] tags Second argument to the constructor function of
     *            the library.
     *
     * \see Module, describing the structure of the modules that can be loaded.
     */
    bool load_module_from_library(Module** target, std::string const& libname,
                                  TFV_Int id);

    /**
     * Destruct the module and dlclose the associated library (once).
     * \param[in] module The module to be free'd.
     */
    bool destroy_module(Module* module);

    /**
     * Destroy all modules and dlclose the associated libraries.
     * Since dlopen/dlclose use an internal refcounting mechanism, the
     * libraries should be closed after this method.
     */
    void destroy_all(void);

    /**
     * Return the last error produced by one of the api methods, if
     * any. This should be called whenever one of destroy_module or
     * load_module_from_library return false.
     * Calling this will also reset the internal error to TFV_OK.
     * \return The last error produced, one of TFV*. TFV_OK if none.
     */
    TFV_Result last_error(void);

private:
    using LibraryHandle = void*;
    struct ModuleHandle {
        std::string libname;
        LibraryHandle handle;
    };
    using Handles = std::unordered_map<Module*, ModuleHandle>;

    Handles handles_;                     ///< keeps track of loaded modules
    std::string const system_load_path_;  ///< default shared object files
    std::string const user_load_path_;    ///< additional shared object files
    TFV_Result error_{TFV_OK};  ///< If an error occurs, it's stored here

    std::vector<std::string> required_functions_ = {
        "create",
        "destroy"};  ///< Each library has to provide these methods globally

    // internally used helper methods
    bool _free_lib(LibraryHandle handle);
    bool _file_exists(std::string const& fullname) const;
    bool _load_module_from_library(Module** target,
                                   std::string const& library_root,
                                   std::string const& libname, TFV_Int id);
};
}
