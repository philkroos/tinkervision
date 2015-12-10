/// \file api.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Declares the internal interface of Tinkervision.
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

#include <chrono>
#include <thread>
#include <mutex>
#include <typeinfo>
#include <limits>
#include <functional>
#include <algorithm>
#include <tuple>
#include <cstring>

#include "strings.hh"
#include "tinkervision_defines.h"
#include "cameracontrol.hh"
#include "image.hh"
#include "scenetrees.hh"
#include "module_loader.hh"
#include "shared_resource.hh"
#include "environment.hh"
#include "logger.hh"

namespace tv {

/// Defines the public api of the Tinkervision library.
class Api {
public:
    /// No copies allowed.
    Api(Api const&) = delete;
    Api(Api const&&) = delete;

    /// No copies allowed.
    Api& operator=(Api const&) = delete;
    Api& operator=(Api const&&) = delete;

    /// Standard d'tor, calls quit().
    ~Api(void);

    /// Should be checked before first usage of the api.
    /// \return true if the library has been constructed successfully.
    bool valid(void) const;

    /// Starts execution of all active modules.  This is only
    /// necessary if the Api had been stopped.  The method is
    /// automatically called during construction of the Api.
    /// \sa stop()
    /// \return #TV_OK if execution started successfully.
    int16_t start(void);

    /// Halts (pauses) execution of the main-loop.  This will not do
    /// any change to the registered modules, only they will stop
    /// being executed.  If calling start(), the Api will resume
    /// execution with the same configuration.
    /// This will also stop a running dummy module.
    /// \sa start_idle()
    /// \note Even while the api is paused there can still be new modules
    /// registered. They will start execution once start() is called.
    /// \sa quit() If the whole thing shall be stopped.
    /// \return A result code:
    ///     - #TV_OK when execution halted successfully.
    ///     - #TV_EXEC_THREAD_FAILURE when the thread is still running.
    int16_t stop(void);

    /// Stops and removes all running modules.  This is not necessary
    /// in general if the Api is being deconstructed in a controlled
    /// way. If, however, the client application should crash or exit
    /// without stopping all instantiated modules, or a complete
    /// restart including reset of the camera is desired, this can be
    /// used.  This will also stop and remove a running dummy module.
    ///
    /// \sa start_idle()
    ///
    /// \sa  start()
    /// \return A result code:
    ///     - #TV_OK when execution halted successfully.
    ///     - #TV_EXEC_THREAD_FAILURE when the thread is still running.
    int16_t quit(void);

    /// Execute a specific module now, interrupting the main execution loop.
    /// - If module is inactive, activate it for one run
    /// - Do not request a new frame
    /// - Halt, but later resume the main execution loop
    /// \param[in] id Id of a loaded module.
    /// \return #TV_NOT_IMPLEMENTED
    int16_t module_run_now(int8_t id);

    /// Execute a specific module now, interrupting the main execution loop.
    /// - If module is inactive, activate it for one run
    /// - Request a new frame
    /// - Stop the current main execution loop
    /// \param[in] id Id of a loaded module.
    /// \return #TV_NOT_IMPLEMENTED
    int16_t module_run_now_new_frame(int8_t id);

    /// Set the framesize.
    /// \return
    /// - #TV_CAMERA_SETTINGS_FAILED if the selected size is not valid.
    /// - #TV_OK else
    int16_t set_framesize(uint16_t width, uint16_t height);

    /// Start an idle process, i.e. a module which will never be
    /// executed.  This is a lightweight module which will not trigger
    /// frame grabbing.  However, once started, it will keep the camera
    /// device blocked so it may be used to hold on the camera handle
    /// even if no 'real' module is running.  This dummy process can
    /// not be referred to since the assigned id is not retreivable by
    /// the user, and it is (currently) not deactivatable unless quit() is
    /// called.  Also, the process will only be started once,
    /// no matter how often this method gets called.
    ///
    /// \return #TV_OK if the process is running afterwards
    int16_t start_idle(void);

    /// Load a module by its basename under the given id.
    int16_t module_load(std::string const& name, int8_t& id);

    /// Deactivate and remove a module.
    /// \return
    ///   - #TV_NOT_IMPLEMENTED if scenes are active
    ///   - #TV_INVALID_ID if the module does not exist
    ///   - #TV_OK if removal succeeded.
    ///
    /// The method will succeed if:
    int16_t module_destroy(int8_t id);

    /// Set a parameter of a module.  T can be int32_t or std::string.
    /// \param[in] module_id Id of a loaded module (may be inactive).
    /// \param[in] parameter Name of the parameter to set.
    /// \param[in] value Value to set.
    /// \return
    ///    - #TV_NO_SUCH_PARAMETER if the parameter does not exist
    ///    - #TV_MODULE_ERROR_SETTING_PARAMETER if the value is incompatible
    ///    - #TV_OK else
    template <typename T>
    int16_t set_parameter(int8_t module_id, std::string const& parameter,
                          T const& value) {

        return modules_->exec_one(module_id, [&](ModuleWrapper& module) {
            if (not module.has_parameter(parameter)) {
                return TV_MODULE_NO_SUCH_PARAMETER;
            }
            if (not module.set_parameter(parameter, value)) {
                return TV_MODULE_ERROR_SETTING_PARAMETER;
            }
            return TV_OK;
        });
    }

    /// Get a parameter's value from a module. T can be int32_t or
    /// std::string.
    /// \param[in] module_id Id of a loaded module (may be inactive).
    /// \param[in] parameter Name of the parameter to get.
    /// \param[out] value Value of parameter.
    ///    - #TV_NO_SUCH_PARAMETER if the parameter does not exist
    ///    - #TV_OK else
    template <typename T>
    int16_t get_parameter(int8_t module_id, std::string const& parameter,
                          T& value) {

        return modules_->exec_one(module_id, [&](ModuleWrapper& module) {
            if (not module.get_parameter(parameter, value)) {
                return TV_MODULE_NO_SUCH_PARAMETER;
            }

            return TV_OK;
        });
    }

    /// Start a module which was already initialized by
    /// module_load().  This method succeeds if the module was
    /// already started or can be started.
    ///
    /// \param[in] id The id of the module to start.
    ///
    /// \return
    /// - #TV_INVALID_ID if no module is registered with
    ///   the given id.
    /// - #TV_CAMERA_ACQUISATION_FAILED if the
    ///   camera is not available
    /// - #TV_OK iff the module is running after returning.
    int16_t module_start(int8_t module_id);

    /// Pause a module. This will not remove the module but rather
    /// prevent it from being executed. The id is still reserved and it's
    /// a matter of calling start_id() to resume execution.  To actually
    /// remove the module, call remove_id().
    /// \note The associated resources (namely the camera handle)
    /// will be released (once) to be usable in other contexts, so
    /// this might prohibit restart of the module. If however the camera
    /// is used by other modules as well, it will stay open.
    ///
    /// \param id The id of the module to stop. The type of the
    ///           associated module has to match Module.
    /// \return
    ///  - #TV_OK if the module was stopped and marked for removal
    ///  - #TV_INVALID_ID if the id is not registered
    int16_t module_stop(int8_t module_id);

    /// Convert Api return code to string.
    /// \param[in] code The return code to be represented as string.
    /// \return The string representing code
    char const* result_string(int16_t code) const;

    /// Check if a camera is available in the system.
    /// \return true if available
    bool is_camera_available(void);

    /// Check if a specific camera is available in the system.
    /// \param[in] id Device id
    /// \return true if available.
    bool is_camera_available(uint8_t id);

    /// Select a specific camera.
    /// If the specified camera is not available in the system, another one may
    /// still be used. This is for the case that multiple cameras are available.
    /// If the library is already active with another camera and the selected
    /// one would be available too, it will be switched.
    /// \param[in] id Device id
    /// \return true if the device is available.
    bool prefer_camera_with_id(uint8_t id);

    /// Retrieve the frame settings from the camera. This can only work
    /// if the camera was opened already
    /// \param[out] width The framewidth in pixels
    /// \param[out] width The frameheight in pixels
    /// \return
    ///  - #TV_CAMERA_NOT_AVAILABLE if the camera is not open
    ///  - #TV_OK else.
    int16_t resolution(uint16_t& width, uint16_t& height);

    /// Set the time between frame grabbing.  This effectively changes
    /// the frequency of module execution.  It is recommended to keep
    /// it at a decent value because the CPU-load can be quite high
    /// with a too low value.  The value set here is the maximum rate,
    /// it may well be that the actual execution is slower.  Retrieve
    /// that value from effective_frameperiod().

    /// \note If  no module is active,  a minimum latency of  200ms is
    /// hardcoded  (with the  value set  here being  used if  larger).
    /// \param ms The duration of a frameperiod in milliseconds.
    /// \return #TV_OK
    int16_t request_frameperiod(uint32_t ms);

    /// Get the name of a module.
    /// \param[in] module_id Id of a loaded module.
    /// \param[out] name Name of the module, which equals its file(base)name.
    /// \return
    ///    - #TV_INVALID_ID if no such module is loaded.
    ///    - #TV_OK else
    int16_t module_get_name(int8_t module_id, std::string& name) const;

    /// Check if a loaded module is active, i.e. actually running.
    /// \param[in] id Id of an active module.
    /// \param[out] active
    /// \return
    ///    - #TV_INVALID_ID if no such module is loaded.
    ///    - #TV_OK else
    int16_t module_is_active(int8_t module_id, bool& active) const;

    /// Retrieve the number of loaded libraries.
    /// The result can be used with module_id().
    /// \return Number of loaded libraries.
    size_t loaded_libraries_count(void) const;

    /// Retrieve the id of a loaded library.
    /// \param[in] library A number \c [0, loaded_libraries_count())
    /// \param[out] id Id of the library.
    /// \return
    ///    - #TV_INVALID_ARGUMENT if the argument is out of range.
    ///    - #TV_OK else
    int16_t module_id(size_t library, int8_t& id) const;

    /// Get the number of parameters available for a library module.
    /// \param[in] libname Name of the library w/o extension.
    /// \param[out] count Number of parameters, can be used with
    /// library_describe_parameter()
    /// \return
    ///    - #TV_INVALID_ARGUMENT if no such library is loadable.
    ///    - #TV_OK else
    int16_t library_get_parameter_count(std::string const& libname,
                                        uint16_t& count) const;

    /// Get the description of a parameter of a library module.
    /// If the parameter is numeric, the min/max/default values are returned. If
    /// the parameter is a string, these are not set. If the function does not
    /// return #TV_OK, no value returned is valid.
    /// \param[in] libname Name of the library w/o extension.
    /// \param[in] parameter Number of the paramater, in range of
    /// library_get_parameter_count()
    /// \param[out] name Name of the parameter, to be used with parameter_set()
    /// \param[out] type 0 = int32_t, 1 = string.
    /// \param[out] min Minimum allowed value of the parameter
    /// \param[out] max Maximum allowed value of the parameter
    /// \param[out] def Default, initial value of the parameter
    /// \return
    ///    - #TV_INVALID_ARGUMENT if no such library is loadable.
    ///    - #TV_OK else
    int16_t library_describe_parameter(std::string const& libname,
                                       size_t parameter, std::string& name,
                                       uint8_t& type, int32_t& min,
                                       int32_t& max, int32_t& def);

    /// Register a callback to enumerate all parameters of a module.
    /// \deprecated Not used in Tinkerforge context.
    int16_t module_enumerate_parameters(int8_t module_id,
                                        TV_StringCallback callback,
                                        void* context) const;

    /// Attach a callback that will be notified about newly or no
    /// longer available, loadable modules.  Initially, all currently
    /// available modules will be listed.
    ///
    /// \note It does not make
    /// sense to add more then one callback since only the latest will
    /// ever be updated.
    ///
    /// \param[in] callback A #TV_LibrariesCallback, where the first
    /// parameter is the library name, the second the load path, the
    /// third the status which is either 1 (library created) or -1
    /// (library deleted).
    /// \param context Additional context to be passed to each callback.
    int16_t libraries_changed_callback(TV_LibrariesCallback callback,
                                       void* context);

    /// Set the user path prefix.
    /// path has to be a valid, accessible filesystem directory and provide
    /// certain subdirectories (\see tinkervision_defines.h).  The standard
    /// prefix can be set during library build \c (USER_PREFIX=... make), and
    /// defaults to \c $HOME/tv.
    /// \param path A valid, accessible full pathname.
    /// \return
    ///    - #TV_INVALID_ARGUMENT if the path could not be set
    ///    - #TV_OK else.
    int16_t set_user_paths_prefix(std::string const& path);

    /// Set a callback to the results of a specific module.
    /// \deprecated This is not usable in the context of Tinkerforge.
    int16_t callback_set(int8_t module_id, TV_Callback callback);

    /// Set a callback to the results of each module.
    /// \param[in] callback Called after each execution of a module provided it
    /// has a result of type TV_ModuleResult
    /// \return #TV_OK
    /// \note If you want to unregister a callback, simply pass a nullptr here.
    /// \todo Implement method unregister_callback in redbrickapid.
    int16_t callback_default(TV_Callback callback);

    /// Get the latest result from a module.
    /// \param[in] module_id Id of an active module.
    /// \param[out] result Result provided by the module.
    /// \note You need to check the return value to see if result is valid.
    /// \return
    ///    - #TV_INVALID_ID if the module is not loaded or inactive
    ///    - #TV_RESULT_NOT_AVAILABLE if the module has no result
    ///    - #TV_OK if result is valid
    int16_t get_result(int8_t module_id, TV_ModuleResult& result);

    /// Retrieve the effective inverse framerate.
    /// \return effective_frameperiod_.
    /// \see request_frameperiod()
    uint32_t effective_frameperiod(void) const;

    /// Retrieve the current user path.
    /// \see set_user_paths_prefix().
    /// \return The path holding the directory structure for user files.
    std::string const& user_paths_prefix(void) const;

    /// Retrieve the system-wide path to the vision modules.
    /// \return Path where vision modules are installed.
    std::string const& system_module_path(void) const;

    /// Disable and remove all modules.
    void remove_all_modules(void);

    /// Get the number of available libraries
    /// \param[out] count
    void get_libraries_count(uint16_t& count) const;

    /// Get name and load path of a library.
    /// \param[in] count In range [0, get_libraries_count())
    /// \param[out] name
    /// \param[out] path
    bool library_get_name_and_path(uint16_t count, std::string& name,
                                   std::string& path) const;

    // Unfinished scene approach

    /// Start a scene which is a directed chain of modules.
    int16_t scene_start(int8_t module_id, int16_t* scene_id) {
        Log("API", "Starting scene");
        return TV_NOT_IMPLEMENTED;

        if (not modules_->managed(module_id)) {
            return TV_INVALID_ID;
        }

        if ((*modules_)[module_id]->tags() &
                ModuleWrapper::Tag::ExecAndRemove or
            (*modules_)[module_id]->tags() & ModuleWrapper::Tag::Removable) {
            return TV_NOT_IMPLEMENTED;
        }

        *scene_id = _next_scene_id();
        return scene_trees_.scene_start(*scene_id, module_id);
    }

    int16_t scene_remove(int16_t scene_id) {
        Log("API", "Removing scene");
        return TV_NOT_IMPLEMENTED;
    }

    int16_t add_to_scene(int16_t scene_id, int16_t module_id) {
        Log("API", "Add to scene: ", module_id, " -> ", scene_id);
        return TV_NOT_IMPLEMENTED;

        if ((*modules_)[module_id]->tags() &
                ModuleWrapper::Tag::ExecAndRemove or
            (*modules_)[module_id]->tags() & ModuleWrapper::Tag::Removable) {
            return TV_NOT_IMPLEMENTED;
        }

        // \todo If adding the scene fails the module has to be in the
        // same state as before.
        auto result = _enable_module(module_id);
        if (result != TV_OK) {
            return result;
        }

        return scene_trees_.add_to_scene(scene_id, module_id);
    }

    int16_t scene_disable(int16_t scene_id) { return TV_NOT_IMPLEMENTED; }

    int16_t scene_enable(int16_t scene_id) { return TV_NOT_IMPLEMENTED; }

private:
    friend tv::Api& get_api(void);  ///< Singleton accessor

    using Modules = tv::SharedResource<tv::ModuleWrapper>;  ///< Instantiation
    /// of the resource manager using the abstract base class of a vision
    /// algorithm.

    Api(void) noexcept(noexcept(CameraControl()) and
                       noexcept(FrameConversions()) and noexcept(Strings()) and
                       noexcept(SceneTrees()));

    std::atomic_flag done_{ATOMIC_FLAG_INIT};  ///< Used to synchronize long
    /// lasting operations, in particular _module_load()

    CameraControl camera_control_;  ///< Camera access abstraction
    FrameConversions conversions_;  ///< Camera frame in requested formats
    Strings result_string_map_;     ///< String mapping of Api-return values
    SceneTrees scene_trees_;

    Environment* environment_;     ///< Configuration and scripting context
    Modules* modules_;             ///< RAII-style managed vision algorithms.
    ModuleLoader* module_loader_;  ///< Manages available libraries

    Image image_;  ///< Current frame in requested format

    bool api_valid_{false};  ///< True once constructed to valid state.
    bool idle_process_running_{false};   ///< Dummy module activated?
    uint32_t effective_frameperiod_{0};  ///< Effective inverse framerate

    std::thread executor_;        ///< Mainloop-Context executing the modules.
    bool active_ = true;          ///< While true, the mainloop is running.
    bool paused_ = false;         ///< Pauses module execution if true
    uint32_t frameperiod_ms_{0};  ///< Minimum inverse framerate

    TV_Callback default_callback_ = nullptr;

    bool active(void) const { return active_; }
    bool active_modules(void) const { return modules_->size(); }

    /// Only context from which modules are executed.
    void module_exec(int16_t id, ModuleWrapper& module);
    friend Modules;

    /// Threaded execution context of vision algorithms (modules).
    /// This method is started asynchronously during construction of
    /// the Api and is running until deconstruction.  It is constantly
    /// grabbing frames from the active camera, executing all active
    /// modules and activating newly registered modules.
    void execute(void);

    int16_t _module_load(std::string const& name, int16_t id);

    void _disable_all_modules(void);

    void _disable_module_if(
        std::function<bool(tv::ModuleWrapper const& module)> predicate);

    void _enable_all_modules(void);

    int16_t _enable_module(int16_t id);

    int16_t _disable_module(int16_t id);

    /// Generate a new module id.
    /// \todo There is currently no code that would prevent regeneration of
    /// an
    /// id that is currently in use in case of an overflow and a long
    /// running
    /// module.
    int16_t _next_public_id(void) const;

    int16_t _next_internal_id(void) const;

    bool _scenes_active(void) const { return not scene_trees_.empty(); }

    int16_t _next_scene_id(void) const {
        static int16_t scene_id{std::numeric_limits<int8_t>::max() + 1};
        return scene_id++;
    }
};
Api& get_api(void);
}
