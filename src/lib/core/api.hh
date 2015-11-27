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
#include <atomic>
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

    /// No copies allowed.
    Api& operator=(Api const&) = delete;

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
    /// \return
    ///  - #TV_CAMERA_NOT_AVAILABLE if the camera is not available,
    ///  - #TV_OK else
    int16_t is_camera_available(void);

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

    /// Get the number of parameters available for a library module.
    /// \param[in] libname Name of the library w/o extension.
    /// \param[out] count Number of parameters, can be used with
    /// library_describe_parameter()
    /// \return
    ///    - #TV_INVALID_ARGUMENT if no such library is loadable.
    ///    - #TV_OK else
    int16_t library_get_parameter_count(std::string const& libname,
                                        size_t& count) const;

    /// Get the description of a parameter of a library module.
    /// \param[in] libname Name of the library w/o extension.
    /// \param[in] parameter Number of the paramater, in range of
    /// library_get_parameter_count()
    /// \param[out] name Name of the parameter, to be used with parameter_set()
    /// \param[out] type 0 = int32_t, 1 = string.
    /// \param[out] string Default value of the parameter
    /// \param[out] min Minimum allowed value of the parameter
    /// \param[out] max Maximum allowed value of the parameter
    /// \param[out] def Default, initial value of the parameter
    /// \return
    ///    - #TV_INVALID_ARGUMENT if no such library is loadable.
    ///    - #TV_OK else
    int16_t library_describe_parameter(std::string const& libname,
                                       size_t parameter, std::string& name,
                                       uint8_t& type, std::string& string,
                                       int32_t& min, int32_t& max,
                                       int32_t& def);

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

    /// Set the path to the loadable modules.  The system path is
    /// fixed to #SYS_MODULE_LOAD_PATH, but the path set here
    /// (defaulting to #ADD_MODULE_LOAD_PATH) is prioritized during module
    /// loading.
    /// \param path A valid, accessible full pathname.
    /// \return #TV_INVALID_ARGUMENT if the path could not be set; #TV_OK
    /// else.
    int16_t set_user_module_load_path(std::string const& path);

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

    std::string const& user_module_path(void) const;

    std::string const& system_module_path(void) const;

    /// Disable and remove all modules.
    void remove_all_modules(void);

    void get_libraries_count(uint16_t& count) const;

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

    CameraControl camera_control_;  ///< Camera access abstraction
    FrameConversions conversions_;  ///< Camera frame in requested formats
    Strings result_string_map_;     ///< String mapping of Api-return values
    Environment* environment_;      ///< Configuration and scripting context
    SceneTrees scene_trees_;

    Modules* modules_;             ///< RAII-style managed vision algorithms.
    ModuleLoader* module_loader_;  ///< Manages available libraries

    bool api_valid_{false};  ///< True once constructed to valid state.
    bool idle_process_running_{false};   ///< Dummy module activated?
    uint32_t effective_frameperiod_{0};  ///< Effective inverse framerate

    std::thread executor_;        ///< Mainloop-Context executing the modules.
    bool active_ = true;          ///< While true, the mainloop is running.
    uint32_t frameperiod_ms_{0};  ///< Minimum inverse framerate

    TV_Callback default_callback_ = nullptr;

    bool active(void) const { return active_; }
    bool active_modules(void) const { return modules_->size(); }

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
