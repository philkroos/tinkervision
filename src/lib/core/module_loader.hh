/// \file module_loader.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of the class \c ModuleLoader.
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

#include <unordered_map>
#include <vector>
#include <dlfcn.h>

#include "module_wrapper.hh"
#include "filesystem.hh"
#include "dirwatch.hh"
#include "environment.hh"

namespace tv {

/// Load external modules, manage and release their so-handles.
/// This class provides a wrapper around the shared object loading
/// code.  Loaded shared objects are managed in an internal map
/// indexed by the address of the actual modules loaded from the
/// libraries.  As a consequence, a library is dlopen'ed once per
/// module, even if it is already opened, and also dlclose'd once.
///
/// \see The manpage for dlopen/dlsym/dlclose.
/// \see SharedResource which is used to lock usage of loaded modules.
/// \see Api::module_load(), from where the instantiated library object
/// is passed into SharedResource.
class ModuleLoader {
public:
    /// c'tor.
    /// Initialize this class with a path where libraries are to be
    /// found.
    ModuleLoader(Environment const& environment_);

    /// d'tor. Call destroy_all() and delete acquired resources.
    ~ModuleLoader(void);

    /// Modify the user accessible module load path.
    /// \param[in] old_path full, previous path.
    /// \param[in] load_path full path name searched for for libraries.
    /// \return true if the path was added or not modified.
    bool switch_user_load_path(std::string const& old_path,
                               std::string const& load_path);

    /// List all loadable modules. Both system_load_path_ and
    /// user_lib_load_path_ are searched for loadable modules, which are
    /// identified by their basename, which equals the module name.
    /// \param[inout] paths The paths to the modules.
    /// \param[inout] modules The filenames of the modules w/o extension.
    void list_available_modules(std::vector<std::string>& paths,
                                std::vector<std::string>& modules) const;

    /// \param[inout] modules List of all modules found.
    /// Load a vision-module from library libname.
    ///
    /// \param[inout] target Pointer to the Module pointer which will
    ///               point to the loaded module.
    /// \param[in] libname basename of the library to be opened.
    /// \param[in] id First argument to the constructor function of the library.
    /// \param[in] tags Second argument to the constructor function of
    ///            the library.
    ///
    /// \see Module, describing the structure of the modules that can be loaded.
    bool load_module_from_library(ModuleWrapper** target,
                                  std::string const& libname, int16_t id);

    /// Destruct the module and dlclose the associated library (once).
    /// \param[in] module The module to be free'd.
    bool destroy_module(ModuleWrapper* module);

    /// Destroy all modules and dlclose the associated libraries.
    /// Since dlopen/dlclose use an internal refcounting mechanism, the
    /// libraries should be closed after this method.
    void destroy_all(void);

    /// Return the last error produced by one of the api methods, if
    /// any. This should be called whenever one of destroy_module or
    /// load_module_from_library return false.
    /// Calling this will also reset the internal error to TV_OK.
    /// \return The last error produced, one of TV*. TV_OK if none.
    int16_t last_error(void);

    /// Set a callback that will be notified for each change to the available
    /// load paths.
    /// \note Only one callback is allowed. If a callback is already registered,
    /// it will be replaced by this function.
    /// \param[in] callback A function accepting a string for the directory, a
    /// string for the filename, and a flag denoting whether the specified file
    /// has been created (true) or deleted (false). The flag is also false if a
    /// watched directory has been deleted. In this case, filename is the empty
    /// string.
    void update_on_changes(std::function<void(std::string const& directory,
                                              std::string const& filename,
                                              Dirwatch::Event event)> callback);

    /// Retrieve the number of currently available libraries.
    /// \return Number of libraries
    size_t libraries_count(void) const;

    /// Retrieve name and load path of an available library.
    /// \param[in] count A number [0, libraries_count())
    /// \param[out] name Library name.
    /// \param[out] path Library load path.
    /// \return True if count is smaller than the size of availables_.
    bool library_name_and_path(size_t count, std::string& name,
                               std::string& path) const;

    /// Check whether a library is available for loading
    /// \param[in] libname Name of the module, i.e. filename w/o extension.
    /// \return True if available.
    bool library_available(std::string const& libname) const;

    /// Get the number of parameters a library supports.
    /// \param[in] libname Name of the module, i.e. filename w/o extension.
    /// \param[out] count Number of supported parameters, unset if libname is
    /// not available!
    /// \return True if the library is available. Only then count is valid.
    bool library_parameter_count(std::string const& libname,
                                 uint16_t& count) const;

    /// Get the properties of a parameter from a library.
    /// \param[in] libname Name of the module, i.e. filename w/o extension.
    /// \param[in] number A number < library_parameter_count()
    /// \param[out] p Requested parameter.
    /// \return False if the library is not available or number is out of range.
    bool library_get_parameter(std::string const& libname, size_t number,
                               Parameter const** p) const;

private:
    struct AvailableModule {
        std::string libname;
        std::string loadpath;
        std::vector<Parameter const*> parameter;
    };
    using AvailableModules = std::vector<AvailableModule>;

    using LibraryHandle = void*;
    struct ModuleHandle {
        std::string libname;
        std::string loadpath;
        LibraryHandle handle;
    };
    using Handles = std::unordered_map<ModuleWrapper*, ModuleHandle>;

    Environment const& environment_;
    AvailableModules availables_;  ///< Keeps track of loadable modules
    Handles handles_;              ///< Keeps track of loaded modules
    int16_t error_{TV_OK};         ///< If an error occurs, it's stored here

    std::vector<std::string> required_functions_ = {
        "create",
        "destroy"};  ///< Each library has to provide these methods globally

    std::function<void(std::string const&, std::string const&,
                       Dirwatch::Event)> on_change_callback =
        nullptr;  ///< Signature of callback for changes in the module load
                  /// directories.

    Dirwatch dirwatch_;  ///< Watch load paths for new or deleted modules.

    // internally used helper methods
    bool _free_lib(LibraryHandle handle);
    LibraryHandle _load_module_from_library(ModuleWrapper** target,
                                            std::string const& library_root,
                                            std::string const& libname,
                                            int16_t id);

    void _watched_directory_changed_callback(Dirwatch::Event event,
                                             std::string const& dir,
                                             std::string const& file);

    bool _add_available_module(std::string const& path,
                               std::string const& name);

    /// List all shared objects in user and system path. These might not be
    /// valid vision modules, which is only checked when adding them to
    /// availables_.
    /// \see list_available_modules(), which returns the content of availables_,
    /// i.e. the actually loadable vision modules.
    void _possibly_available_modules(std::vector<std::string>& paths,
                                     std::vector<std::string>& modules) const;
};
}
