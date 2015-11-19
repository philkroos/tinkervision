/// \file api.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief The internal interface of the Vision library is declared (and
///        partly defined) here.
///
/// The interface is provided by the class Api.
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

#include "window.hh"
#include "logger.hh"

namespace tv {

/// Defines the public api of the Tinkervision library.
class Api {
private:
    Api(void);
    friend tv::Api& get_api(void);
    bool active(void) const { return active_; }
    bool active_modules(void) const { return modules_.size(); }

public:
    Api(Api const&) = delete;
    Api& operator=(Api const&) = delete;

    ~Api(void);

    /// Starts execution of all active modules.  This is only
    /// necessary if the Api had been stopped.  The method is
    /// automatically called during construction of the Api.
    /// \sa stop()
    /// \return #TV_OK if execution started successfully.
    TV_Result start(void);

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
    TV_Result stop(void);

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
    TV_Result quit(void);

    /// Set the framesize.
    /// \return
    /// - #TV_CAMERA_SETTINGS_FAILED if the selected size is not valid.
    /// - #TV_OK else
    TV_Result set_framesize(uint16_t width, uint16_t height);

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
    TV_Result start_idle(void);

    /// Load a module by its basename under the given id.
    TV_Result module_load(std::string const& name, TV_Id& id);

    /// Deactivate and remove a module.
    /// \return
    ///   - #TV_NOT_IMPLEMENTED if scenes are active
    ///   - #TV_INVALID_ID if the module does not exist
    ///   - #TV_OK if removal succeeded.
    ///
    /// The method will succeed if:
    TV_Result module_destroy(TV_Id id);

    TV_Result set_parameter(TV_Id module_id, std::string parameter,
                            parameter_t value);

    TV_Result get_parameter(TV_Id module_id, std::string parameter,
                            parameter_t* value);

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
    TV_Result module_start(TV_Id module_id);

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
    TV_Result module_stop(TV_Id module_id);

    /// Convert Api return code to string.
    /// \param[in] code The return code to be represented as string.
    /// \return The string representing code
    TV_String result_string(TV_Result code) const;

    /// Check if a camera is available in the system.
    /// \return
    ///  - #TV_CAMERA_NOT_AVAILABLE if the camera is not available,
    ///  - #TV_OK else
    TV_Result is_camera_available(void);

    /// Retrieve the frame settings from the camera. This can only work
    /// if
    /// the
    /// camera was opened already
    /// \param[out] width The framewidth in pixels
    /// \param[out] width The frameheight in pixels
    /// \return
    ///  - #TV_CAMERA_NOT_AVAILABLE if the camera is not open
    ///  - #TV_OK else.
    TV_Result resolution(TV_Size& width, TV_Size& height);

    /// Set the time between the execution of active modules.
    /// This is set to a default of 100ms, meaning that the mainloop
    /// pauses for half a second between two executions of the active
    /// modules. It is recommended to keep it at a decent value
    /// because the CPU-load can be quite high with a too low value.
    /// However, with a lot of active modules, this will reduce
    /// respondability.
    /// \note If no module is active, a minimum latency of 200ms is
    /// hardcoded (with the value set here being used if larger).
    /// \param ms The duration of the pauses in milliseconds.
    TV_Result set_execution_latency_ms(TV_UInt ms);

    TV_Result module_get_name(TV_Id module_id, std::string& name) const;

    TV_Result library_get_parameter_count(std::string const& libname,
                                          size_t& count) const;

    TV_Result library_describe_parameter(std::string const& libname,
                                         size_t parameter, std::string& name,
                                         parameter_t& min, parameter_t& max,
                                         parameter_t& def);

    TV_Result module_enumerate_parameters(TV_Id module_id,
                                          TV_StringCallback callback,
                                          TV_Context context) const;

    /// Attach a callback that will be notified about newly or no longer
    /// available, loadable modules.
    /// Initially, all currently available modules will be listed.
    /// \note It does not make sense to add more then one callback since
    /// only
    /// the latest will ever be updated.
    /// \param callback A #TV_StringCallback where the first parameter
    /// denotes
    /// whether the corresponding file has been deleted (0) or is available
    /// (1).
    /// \param context Additional context to be passed to each callback.
    TV_Result libraries_changed_callback(TV_LibrariesCallback callback,
                                         TV_Context context);

    /// Set the path to the loadable modules.  The system path is
    /// fixed to #SYS_MODULE_LOAD_PATH, but the path set here
    /// (defaulting to #ADD_MODULE_LOAD_PATH) is prioritized during module
    /// loading.
    /// \param path A valid, accessible full pathname.
    /// \return #TV_INVALID_ARGUMENT if the path could not be set; #TV_OK
    /// else.
    TV_Result set_user_module_load_path(std::string const& path);

    TV_Result callback_set(TV_Id module_id, TV_Callback callback);

    TV_Result callback_default(TV_Callback callback);

    TV_Result get_result(TV_Id module_id, TV_ModuleResult& result);

    uint32_t effective_framerate(void) const;

    std::string const& user_module_path(void) const;

    std::string const& system_module_path(void) const;

    /// Disable and remove all modules.
    void remove_all_modules(void);

    void get_libraries_count(uint16_t& count) const;

    bool library_get_name_and_path(uint16_t count, std::string& name,
                                   std::string& path) const;

    // Unfinished scene approach

    /// Start a scene which is a directed chain of modules.
    TV_Result scene_start(TV_Id module_id, TV_Scene* scene_id) {
        Log("API", "Starting scene");
        return TV_NOT_IMPLEMENTED;

        if (not modules_.managed(module_id)) {
            return TV_INVALID_ID;
        }

        if (modules_[module_id]->tags() & ModuleWrapper::Tag::ExecAndRemove or
            modules_[module_id]->tags() & ModuleWrapper::Tag::Removable) {
            return TV_NOT_IMPLEMENTED;
        }

        *scene_id = _next_scene_id();
        return scene_trees_.scene_start(*scene_id, module_id);
    }

    TV_Result scene_remove(TV_Scene scene_id) {
        Log("API", "Removing scene");
        return TV_NOT_IMPLEMENTED;
    }

    TV_Result add_to_scene(TV_Scene scene_id, TV_Int module_id) {
        Log("API", "Add to scene: ", module_id, " -> ", scene_id);
        return TV_NOT_IMPLEMENTED;

        if (modules_[module_id]->tags() & ModuleWrapper::Tag::ExecAndRemove or
            modules_[module_id]->tags() & ModuleWrapper::Tag::Removable) {
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

    TV_Result scene_disable(TV_Scene scene_id) { return TV_NOT_IMPLEMENTED; }

    TV_Result scene_enable(TV_Scene scene_id) { return TV_NOT_IMPLEMENTED; }

private:
    CameraControl camera_control_;  ///< Camera access abstraction
    FrameConversions conversions_;
    TVStringMap result_string_map_;     ///< String mapping of Api-return values
    bool idle_process_running_{false};  ///< Dummy module activated?
    double effective_framerate_{0};     ///< Effective inverse framerate

    ModuleLoader module_loader_{SYS_MODULE_LOAD_PATH, ADD_MODULE_LOAD_PATH};

    using Modules = tv::SharedResource<tv::ModuleWrapper>;  ///< Instantiation
    /// of the resource manager using the abstract base class of a vision
    /// algorithm.

    Modules modules_;  ///< RAII-style managed vision algorithms.

    std::thread executor_;  ///< Mainloop-Context executing the modules.
    bool active_ = true;    ///< While true, the mainloop is running.
    unsigned execution_latency_ms_ = 100;  ///< Pause during mainloop

    SceneTrees scene_trees_;

    TV_Callback default_callback_ = nullptr;

    /// Threaded execution context of vision algorithms (modules).
    /// This method is started asynchronously during construction of
    /// the Api and is running until deconstruction.  It is constantly
    /// grabbing frames from the active camera, executing all active
    /// modules and activating newly registered modules.
    void execute(void);

    TV_Result _module_load(std::string const& name, TV_Int id);

    void _disable_all_modules(void);

    void _disable_module_if(
        std::function<bool(tv::ModuleWrapper const& module)> predicate);

    void _enable_all_modules(void);

    TV_Result _enable_module(TV_Int id);

    TV_Result _disable_module(TV_Int id);

    /// Generate a new module id.
    /// \todo There is currently no code that would prevent regeneration of
    /// an
    /// id that is currently in use in case of an overflow and a long
    /// running
    /// module.
    TV_Int _next_public_id(void) const;

    TV_Int _next_internal_id(void) const;

    bool _scenes_active(void) const { return not scene_trees_.empty(); }

    TV_Scene _next_scene_id(void) const {
        static TV_Scene scene_id{std::numeric_limits<TV_Id>::max() + 1};
        return scene_id++;
    }
};

Api& get_api(void);
}
