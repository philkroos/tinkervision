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
    TV_Result set_framesize(TV_Size width, TV_Size height) {

        auto result = TV_CAMERA_SETTINGS_FAILED;

        /// \todo Count the active modules continuously and remove code dup.
        auto active_count = modules_.count(
            [](tv::ModuleWrapper const& module) { return module.enabled(); });

        if (active_count) {
            uint16_t w, h;
            camera_control_.get_resolution(w, h);

            if (w != width or h != height) {  // different settings?
                auto code = stop();
                if (code != TV_OK) {
                    LogError("API", "SetFramesize ", "Stop returned ", code);
                }

                /// If the settings can't be applied, any previous ones will
                /// be restored.
                if (camera_control_.preselect_framesize(width, height)) {
                    result = TV_OK;
                }

                code = start();
                if (code != TV_OK) {
                    LogError("API", "SetFramesize ", "Start returned ", code);
                }
            } else {
                result = TV_OK;
            }
        } else {  // camera not running
            if (camera_control_.preselect_framesize(width, height)) {
                result = TV_OK;
            }
        }

        return result;
    }

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
    TV_Result start_idle(void) {
        auto result = TV_OK;  // optimistic because startable only once

        if (not idle_process_running_) {
            result = _module_load("dummy", _next_internal_id());
        }
        idle_process_running_ = (result == TV_OK);
        return result;
    }

    /// Load a module by its basename under the given id.
    TV_Result module_load(std::string const& name, TV_Id& id) {
        auto module_id = _next_public_id();

        assert(module_id < std::numeric_limits<TV_Id>::max() and module_id > 0);

        auto result = _module_load(name, static_cast<TV_Int>(module_id));

        if (TV_INVALID_ID == result) {
            // this is an unhandled id clash, see _next_public_id
            result = TV_INTERNAL_ERROR;
        }

        if (TV_OK == result) {
            id = static_cast<TV_Id>(module_id);
        }
        return result;
    }

    /// Deactivate and remove a module.
    /// \return
    ///   - #TV_NOT_IMPLEMENTED if scenes are active
    ///   - #TV_INVALID_ID if the module does not exist
    ///   - #TV_OK if removal succeeded.
    ///
    /// The method will succeed if:
    TV_Result module_destroy(TV_Id id) {
        Log("API", "Destroying module ", id);

        /// - no scenes are active (currently). Then,
        if (_scenes_active()) {
            return TV_NOT_IMPLEMENTED;
        }

        /// the module will be disabled and registered for removal which
        /// will
        /// happen in the main execution loop.
        /// \todo Is a two-stage-removal process still necessary now that
        /// the allocation stage was removed from SharedResource? Not sure,
        /// probably not.
        return modules_.exec_one(id, [this](tv::ModuleWrapper& module) {
            module.disable();
            module.tag(ModuleWrapper::Tag::Removable);
            camera_control_.release();
            return TV_OK;
        });
    }

    TV_Result set_parameter(TV_Id module_id, std::string parameter,
                            TV_Word value) {

        return modules_.exec_one(module_id, [&](ModuleWrapper& module) {
            if (not module.has_parameter(parameter)) {
                return TV_MODULE_NO_SUCH_PARAMETER;
            }
            if (not module.set_parameter(parameter, value)) {
                return TV_MODULE_ERROR_SETTING_PARAMETER;
            }
            return TV_OK;
        });
    }

    TV_Result get_parameter(TV_Id module_id, std::string parameter,
                            parameter_t* value) {

        return modules_.exec_one(module_id, [&](ModuleWrapper& module) {
            if (not module.get_parameter(parameter, *value)) {
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
    TV_Result module_start(TV_Id module_id) {
        auto id = static_cast<TV_Int>(module_id);

        if (not modules_.managed(id)) {
            return TV_INVALID_ID;
        }

        return _enable_module(module_id);
    }

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
    TV_Result module_stop(TV_Id module_id) {
        Log("API", "Stopping module ", module_id);

        auto id = static_cast<TV_Int>(module_id);

        if (not modules_.managed(id)) {
            return TV_INVALID_ID;
        }

        return _disable_module(module_id);
    }

    /// Convert Api return code to string.
    /// \param[in] code The return code to be represented as string.
    /// \return The string representing code
    TV_String result_string(TV_Result code) const {
        return result_string_map_[code];
    }

    /// Check if a camera is available in the system.
    /// \return
    ///  - #TV_CAMERA_NOT_AVAILABLE if the camera is not available,
    ///  - #TV_OK else
    TV_Result is_camera_available(void) {
        return camera_control_.is_available() ? TV_OK : TV_CAMERA_NOT_AVAILABLE;
    }

    /// Retrieve the frame settings from the camera. This can only work
    /// if
    /// the
    /// camera was opened already
    /// \param[out] width The framewidth in pixels
    /// \param[out] width The frameheight in pixels
    /// \return
    ///  - #TV_CAMERA_NOT_AVAILABLE if the camera is not open
    ///  - #TV_OK else.
    TV_Result resolution(TV_Size& width, TV_Size& height) {
        return camera_control_.get_resolution(width, height)
                   ? TV_OK
                   : TV_CAMERA_NOT_AVAILABLE;
    }

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
    TV_Result set_execution_latency_ms(TV_UInt ms) {
        execution_latency_ms_ = std::min(TV_UInt(20), ms);
        return TV_OK;
    }

    /// Start a scene which is a directed chain of modules.
    TV_Result scene_start(TV_Id module_id, TV_Scene* scene_id) {
        Log("API", "Starting scene");

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

    TV_Result module_get_name(TV_Id module_id, std::string& name) const {
        if (not modules_.managed(module_id)) {
            return TV_INVALID_ID;
        }

        name = modules_[module_id].name();
        return TV_OK;
    }

    TV_Result library_get_parameter_count(std::string const& libname,
                                          size_t& count) const {
        if (module_loader_.library_parameter_count(libname, count)) {
            return TV_OK;
        }
        return TV_INVALID_ARGUMENT;
    }

    TV_Result library_describe_parameter(std::string const& libname,
                                         size_t parameter, std::string& name,
                                         parameter_t& min, parameter_t& max,
                                         parameter_t& def) {

        if (not module_loader_.library_describe_parameter(
                libname, parameter, name, min, max, def)) {
            return TV_INVALID_ARGUMENT;
        }
        return TV_OK;
    }

    TV_Result module_enumerate_parameters(TV_Id module_id,
                                          TV_StringCallback callback,
                                          TV_Context context) const {
        if (not modules_.managed(module_id)) {
            return TV_INVALID_ID;
        }

        std::vector<Parameter> parameters;
        modules_[module_id].get_parameters_list(parameters);

        if (parameters.size()) {
            std::thread([module_id, parameters, callback, context](void) {
                            for (auto const& par : parameters) {
                                callback(module_id, par.name().c_str(),
                                         context);
                            }
                            callback(0, "", context);  // done
                        }).detach();
        }

        return TV_OK;
    }

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
    TV_Result enumerate_available_modules(TV_StringCallback callback,
                                          TV_Context context) {
        std::thread([&, callback, context](void) {
                        std::vector<std::string> paths, modules;
                        module_loader_.list_available_modules(paths, modules);
                        for (size_t i = 0; i < modules.size(); ++i) {
                            callback(static_cast<int>(true),
                                     (modules[i]).c_str(), context);
                        }
                        LogDebug("API", "Module enumeration finished");
                    }).detach();

        module_loader_.update_on_changes([callback, context](
            std::string const& dir, std::string const& file, bool created) {
            callback(static_cast<int>(created), (dir + file).c_str(), context);
        });
        return TV_OK;
    }

    /// Set the path to the loadable modules.  The system path is
    /// fixed to #SYS_MODULE_LOAD_PATH, but the path set here
    /// (defaulting to #ADD_MODULE_LOAD_PATH) is prioritized during module
    /// loading.
    /// \param path A valid, accessible full pathname.
    /// \return #TV_INVALID_ARGUMENT if the path could not be set; #TV_OK
    /// else.
    TV_Result set_user_module_load_path(std::string const& path) {
        return module_loader_.set_user_load_path(path) ? TV_OK
                                                       : TV_INVALID_ARGUMENT;
    }

    TV_Result callback_set(TV_Id module_id, TV_Callback callback) {
        if (default_callback_ != nullptr) {
            return TV_GLOBAL_CALLBACK_ACTIVE;
        }

        if (not modules_[module_id]) {
            return TV_INVALID_ID;
        }

        auto& module = *modules_[module_id];

        if (not module.register_callback(callback)) {
            LogError("API", "Could not set callback for module ",
                     module.name());
            return TV_INTERNAL_ERROR;
        }

        return TV_OK;
    }

    TV_Result callback_default(TV_Callback callback) {
        default_callback_ = callback;
        return TV_OK;
    }

    TV_Result get_result(TV_Id module_id, TV_ModuleResult& result) {
        Log("API", "Getting result from module ", module_id);

        return modules_.exec_one(module_id, [&](ModuleWrapper& module) {
            auto res = module.result();
            if (res) {
                return TV_RESULT_NOT_AVAILABLE;
            }

            result.x = res.x;
            result.y = res.y;
            result.width = res.width;
            result.height = res.height;
            std::strncpy(result.string, res.result.c_str(),
                         TV_CHAR_ARRAY_SIZE - 1);
            std::fill(result.string + res.result.size(),
                      result.string + TV_CHAR_ARRAY_SIZE - 1, '\0');
            result.string[TV_CHAR_ARRAY_SIZE - 1] = '\0';
            return TV_OK;
        });
    }

    double effective_framerate(void) const { return effective_framerate_; }

    std::string const& user_module_path(void) const {
        return module_loader_.user_load_path();
    }

    std::string const& system_module_path(void) const {
        return module_loader_.system_load_path();
    }

    /// Disable and remove all modules.
    void remove_all_modules(void) {
        _disable_all_modules();

        modules_.free_all();
        idle_process_running_ = false;
        Log("Api", "All modules released");
    }

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

    TV_Result _module_load(std::string const& name, TV_Int id) {
        Log("API", "ModuleLoad ", name, " ", id);

        if (modules_[id]) {
            return TV_INVALID_ID;
        }

        auto module = (ModuleWrapper*)(nullptr);
        if (not module_loader_.load_module_from_library(&module, name, id)) {
            Log("API", "Loading library ", name, " failed");
            return module_loader_.last_error();
        }

        /// \todo Catch the cases in which this fails and remove the module.
        (void)module->initialize();

        if (not camera_control_.acquire()) {
            return TV_CAMERA_NOT_AVAILABLE;
        }

        if (not modules_.insert(id, module, [this](ModuleWrapper& module) {

                module_loader_.destroy_module(&module);
            })) {

            camera_control_.release();
            return TV_MODULE_INITIALIZATION_FAILED;
        }

        /// \todo Catch the cases in which this fails and remove the module.
        (void)module->enable();
        return TV_OK;
    }

    void _disable_all_modules(void) {
        modules_.exec_all([this](TV_Int id, tv::ModuleWrapper& module) {
            module.disable();
            camera_control_.release();
        });
    }

    void _disable_module_if(
        std::function<bool(tv::ModuleWrapper const& module)> predicate) {

        modules_.exec_all(
            [this, &predicate](TV_Int id, tv::ModuleWrapper& module) {
                if (predicate(module)) {
                    module.disable();
                    camera_control_.release();
                }
            });
    }

    void _enable_all_modules(void) {
        modules_.exec_all([this](TV_Int id, tv::ModuleWrapper& module) {
            if (not module.enabled()) {
                if (camera_control_.acquire()) {
                    module.enable();
                }
            }
        });
    }

    TV_Result _enable_module(TV_Int id) {
        return modules_.exec_one(id, [this](tv::ModuleWrapper& module) {
            if (module.enabled() or camera_control_.acquire()) {
                module.enable();  // possibly redundant
                return TV_OK;
            } else {
                return TV_CAMERA_NOT_AVAILABLE;
            }
        });
    }

    TV_Result _disable_module(TV_Int id) {
        return modules_.exec_one(id, [this](tv::ModuleWrapper& module) {
            module.disable();
            camera_control_.release();
            return TV_OK;
        });
    }

    bool _scenes_active(void) const { return not scene_trees_.empty(); }

    /// Generate a new module id.
    /// \todo There is currently no code that would prevent regeneration of
    /// an
    /// id that is currently in use in case of an overflow and a long
    /// running
    /// module.
    TV_Int _next_public_id(void) const {
        static TV_Id public_id{0};
        if (++public_id == 0) {
            public_id = 1;
            LogWarning("API", "Overflow of public ids");
        }
        return static_cast<TV_Int>(public_id);
    }

    TV_Int _next_internal_id(void) const {
        static TV_Int internal_id{std::numeric_limits<TV_Id>::max() + 1};
        return internal_id++;
    }

    TV_Scene _next_scene_id(void) const {
        static TV_Scene scene_id{std::numeric_limits<TV_Id>::max() + 1};
        return scene_id++;
    }
};

Api& get_api(void);
}
