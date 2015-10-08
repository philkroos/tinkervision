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

/** \file api.hh

    The internal interface of the Vision library is declared (and
    partly defined) here.

    The interface is provided by the class Api.
*/

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

namespace tfv {

/** Defines the public api of the Tinkervision library.
 */
class Api {
private:
    Api(void);
    friend tfv::Api& get_api(void);
    bool active(void) const { return active_; }
    bool active_modules(void) const { return modules_.size(); }

public:
    Api(Api const&) = delete;
    Api& operator=(Api const&) = delete;

    ~Api(void);

    /**
     * Starts execution of all active modules.  This is only
     * necessary if the Api had been stopped.  The method is
     * automatically called during construction of the Api.
     * \sa stop()
     * \return #TFV_OK if execution started successfully.
     */
    TFV_Result start(void);

    /**
     * Halts (pauses) execution of the main-loop.  This will not do
     * any change to the registered modules, only they will stop
     * being executed.  If calling start(), the Api will resume
     * execution with the same configuration.
     * This will also stop a running dummy module.
     * \sa start_idle()
     * \note Even while the api is paused there can still be new modules
     * registered. They will start execution once start() is called.
     * \sa quit() If the whole thing shall be stopped.
     * \return A result code:
     *     - #TFV_OK when execution halted successfully.
     *     - #TFV_EXEC_THREAD_FAILURE when the thread is still running.
     */
    TFV_Result stop(void);

    /**
     * Stops and removes all running modules.  This is not necessary
     * in general if the Api is being deconstructed in a controlled
     * way. If, however, the client application should crash or exit
     * without stopping all instantiated modules, or a complete
     * restart including reset of the camera is desired, this can be
     * used.  This will also stop and remove a running dummy module.
     *
     * \sa start_idle()
     *
     * \sa  start()
     * \return A result code:
     *     - #TFV_OK when execution halted successfully.
     *     - #TFV_EXEC_THREAD_FAILURE when the thread is still running.
     */
    TFV_Result quit(void);

    /**
     * Preselect the framesize. This can only be successfull if the camera is
     * not active currently, i.e. if the API is in a stopped state and no module
     * is active.
     * \return
     * - #TFV_CAMERA_SETTINGS_FAILED if there are active modules already
     * - #TFV_OK else
     */
    TFV_Result preselect_framesize(TFV_Size width, TFV_Size height) {
        return camera_control_.preselect_framesize(width, height)
                   ? TFV_OK
                   : TFV_CAMERA_SETTINGS_FAILED;
    }

    /**
     * Start an idle process, i.e. a module which will never be
     * executed.  This is a lightweight module which will not trigger
     * frame grabbing.  However, once started, it will keep the camera
     * device blocked so it may be used to hold on the camera handle
     * even if no 'real' module is running.  This dummy process can
     * not be referred to since the assigned id is not retreivable by
     * the user, and it is (currently) not deactivatable unless quit() is
     * called.  Also, the process will only be started once,
     * no matter how often this method gets called.
     *
     * \return #TFV_OK if the process is running afterwards
     */
    TFV_Result start_idle(void) {
        auto result = TFV_OK;  // optimistic because startable only once

        if (not idle_process_running_) {
            result = _module_load("dummy", _next_internal_id());
        }
        idle_process_running_ = (result == TFV_OK);
        return result;
    }

    /// Load a module by its basename under the given id.
    TFV_Result module_load(std::string const& name, TFV_Id& id) {
        auto module_id = _next_public_id();

        assert(module_id < std::numeric_limits<TFV_Id>::max() and
               module_id > 0);

        auto result = _module_load(name, static_cast<TFV_Int>(module_id));

        if (TFV_INVALID_ID == result) {
            // this is an unhandled id clash, see _next_public_id
            result = TFV_INTERNAL_ERROR;
        }

        if (TFV_OK == result) {
            id = static_cast<TFV_Id>(module_id);
        }
        return result;
    }

    /** Deactivate and remove a module.
     * \return
     *   - #TFV_NOT_IMPLEMENTED if scenes are active
     *   - #TFV_INVALID_ID if the module does not exist
     *   - #TFV_OK if removal succeeded.
     *
     * The method will succeed if:
     */
    TFV_Result module_destroy(TFV_Id id) {
        Log("API", "Destroying module ", id);

        /// - no scenes are active (currently). Then,
        if (_scenes_active()) {
            return TFV_NOT_IMPLEMENTED;
        }

        /// the module will be disabled and registered for removal which will
        /// happen in the main execution loop.
        /// \todo Is a two-stage-removal process still necessary now that
        /// the allocation stage was removed from SharedResource? Not sure,
        /// probably not.
        return modules_.exec_one(id, [this](tfv::Module& module) {
            module.disable();
            module.tag(Module::Tag::Removable);
            camera_control_.release();
            return TFV_OK;
        });
    }

    TFV_Result set_parameter(TFV_Id module_id, std::string parameter,
                             TFV_Word value) {

        return modules_.exec_one(module_id, [&](Module& module) {
            if (not module.has_parameter(parameter)) {
                return TFV_MODULE_NO_SUCH_PARAMETER;
            }
            if (not module.set_parameter(parameter, value)) {
                return TFV_MODULE_ERROR_SETTING_PARAMETER;
            }
            return TFV_OK;
        });
    }

    TFV_Result get_parameter(TFV_Id module_id, std::string parameter,
                             TFV_Word* value) {

        return modules_.exec_one(module_id, [&](Module& module) {
            if (not module.has_parameter(parameter)) {
                return TFV_MODULE_NO_SUCH_PARAMETER;
            }

            module.get_parameter(parameter, *value);

            return TFV_OK;
        });
    }

    /**
     * Start a module which was already initialized by
     * module_load().  This method succeeds if the module was
     * already started or can be started.
     *
     * \param[in] id The id of the module to start.
     *
     * \return
     * - #TFV_UNCONFIGURED_ID if no module is registered with
     *   the given id.
     * - #TFV_CAMERA_ACQUISATION_FAILED if the
     *   camera specified for the module is not available
     * - #TFV_OK iff the module is running after returning.
     */
    TFV_Result module_start(TFV_Id module_id) {
        auto id = static_cast<TFV_Int>(module_id);

        if (not modules_.managed(id)) {
            return TFV_UNCONFIGURED_ID;
        }

        return _enable_module(module_id);
    }

    /**
     * Pause a module. This will not remove the module but rather
     * prevent it from being executed. The id is still reserved and it's
     * a matter of calling start_id() to resume execution.  To actually
     * remove the module, call remove_id().
     * \note The associated resources (namely the camera handle)
     * will be released (once) to be usable in other contexts, so
     * this might prohibit restart of the module. If however the camera
     *is
     * used by other modules as well, it will stay open.
     *
     * \param id The id of the module to stop. The type of the
     *associated
     * module has to match Module.
     * \return
     *  - #TFV_OK if the module was stopped and marked for removal
     *  - #TFV_UNCONFIGURED_ID if the id is not registered
     */
    TFV_Result module_stop(TFV_Id module_id) {
        Log("API", "Stopping module ", module_id);

        auto id = static_cast<TFV_Int>(module_id);

        if (not modules_.managed(id)) {
            return TFV_UNCONFIGURED_ID;
        }

        return _disable_module(module_id);
    }

    /**
     * Convert Api return code to string.
     * \param[in] code The return code to be represented as string.
     * \return The string representing code
     */
    TFV_String result_string(TFV_Result code) const {
        return result_string_map_[code];
    }

    /**
     * Check if a camera is available in the system.
     * \return
     *  - #TFV_CAMERA_ACQUISITION_FAILED if the camera is not available,
     *  - #TFV_OK else
     */
    TFV_Result is_camera_available(void) {
        return camera_control_.is_available() ? TFV_OK
                                              : TFV_CAMERA_ACQUISITION_FAILED;
    }

    /**
     * Retrieve the frame settings from the camera. This can only work
     * if
     * the
     * camera was opened already
     * \param[out] width The framewidth in pixels
     * \param[out] width The frameheight in pixels
     * \return
     *  - #TFV_CAMERA_NOT_AVAILABLE if the camera is not open
     *  - #TFV_OK else.
     */
    TFV_Result resolution(TFV_Size& width, TFV_Size& height) {
        return camera_control_.get_resolution(width, height)
                   ? TFV_OK
                   : TFV_CAMERA_NOT_AVAILABLE;
    }

    /**
     * Set the time between the execution of active modules.
     * This is set to a default of 100ms, meaning that the mainloop
     * pauses for half a second between two executions of the active
     * modules. It is recommended to keep it at a decent value
     * because the CPU-load can be quite high with a too low value.
     * However, with a lot of active modules, this will reduce
     * respondability.
     * \note If no module is active, a minimum latency of 200ms is
     * hardcoded (with the value set here being used if larger).
     * \param ms The duration of the pauses in milliseconds.
     */
    TFV_Result set_execution_latency_ms(TFV_UInt ms) {
        execution_latency_ms_ = std::min(TFV_UInt(20), ms);
        return TFV_OK;
    }

    /**
     * Start a scene which is a directed chain of modules.
     */
    TFV_Result scene_start(TFV_Id module_id, TFV_Scene* scene_id) {
        Log("API", "Starting scene");

        if (not modules_.managed(module_id)) {
            return TFV_INVALID_ID;
        }

        if (modules_[module_id]->tags() & Module::Tag::ExecAndRemove) {
            return TFV_NOT_IMPLEMENTED;
        }

        *scene_id = _next_scene_id();
        return scene_trees_.scene_start(*scene_id, module_id);
    }

    TFV_Result scene_remove(TFV_Scene scene_id) {
        Log("API", "Removing scene");
        return TFV_NOT_IMPLEMENTED;
    }

    TFV_Result add_to_scene(TFV_Scene scene_id, TFV_Int module_id) {
        Log("API", "Add to scene: ", module_id, " -> ", scene_id);

        if (modules_[module_id]->tags() & Module::Tag::ExecAndRemove) {
            return TFV_NOT_IMPLEMENTED;
        }

        // \todo If adding the scene fails the module has to be in the
        // same state as before.
        auto result = _enable_module(module_id);
        if (result != TFV_OK) {
            return result;
        }

        return scene_trees_.add_to_scene(scene_id, module_id);
    }

    TFV_Result scene_disable(TFV_Scene scene_id) { return TFV_NOT_IMPLEMENTED; }

    TFV_Result scene_enable(TFV_Scene scene_id) { return TFV_NOT_IMPLEMENTED; }

    TFV_Result module_get_name(TFV_Id module_id, std::string& name) const {
        if (not modules_.managed(module_id)) {
            return TFV_UNCONFIGURED_ID;
        }

        name = modules_[module_id].name();
        return TFV_OK;
    }

    TFV_Result module_enumerate_parameters(TFV_Id module_id,
                                           TFV_StringCallback callback) const {
        if (not modules_.managed(module_id)) {
            return TFV_UNCONFIGURED_ID;
        }

        std::vector<std::string> parameters;
        modules_[module_id].get_parameters_list(parameters);

        if (parameters.size()) {
            std::thread([module_id, parameters, callback](void) {
                            for (auto const& par : parameters) {
                                callback(module_id, par.c_str());
                            }
                            callback(module_id, "");
                        }).detach();
        }

        return TFV_OK;
    }

    TFV_Result enumerate_available_modules(TFV_StringCallback callback) {
        std::thread([&, callback](void) {
                        std::vector<std::string> modules;
                        module_loader_.list_available_modules(modules);
                        for (auto const& module : modules) {
                            callback(0, module.c_str());
                        }
                        callback(0, "");
                    }).detach();

        return TFV_OK;
    }

    TFV_Result callback_set(TFV_Id module_id, TFV_Callback callback) {
        if (default_callback_ != nullptr) {
            return TFV_GLOBAL_CALLBACK_ACTIVE;
        }

        if (not modules_[module_id]) {
            return TFV_UNCONFIGURED_ID;
        }

        auto& module = *modules_[module_id];

        if (not module.register_callback(callback)) {
            LogError("API", "Could not set callback for module ",
                     module.name());
            return TFV_INTERNAL_ERROR;
        }

        return TFV_OK;
    }

    TFV_Result callback_default(TFV_Callback callback) {
        default_callback_ = callback;
        return TFV_OK;
    }

    TFV_Result get_result(TFV_Id module_id, TFV_ModuleResult& result) {
        Log("API", "Getting result from module ", module_id);

        return modules_.exec_one(module_id, [&](Module& module) {
            auto res = module.result();
            if (res == nullptr) {
                return TFV_RESULT_NOT_AVAILABLE;
            }
            result.x = res->x;
            result.y = res->y;
            result.width = res->width;
            result.height = res->height;
            std::strncpy(result.string, res->result.c_str(),
                         TFV_CHAR_ARRAY_SIZE - 1);
            std::fill(result.string + res->result.size(),
                      result.string + TFV_CHAR_ARRAY_SIZE - 1, '\0');
            result.string[TFV_CHAR_ARRAY_SIZE - 1] = '\0';
            return TFV_OK;
        });
    }

private:
    CameraControl camera_control_;  ///< Camera access abstraction
    FrameConversions conversions_;
    TFVStringMap result_string_map_;    ///< String mapping of Api-return values
    bool idle_process_running_{false};  ///< Dummy module activated?

    ModuleLoader module_loader_{SYS_MODULE_LOAD_PATH, ADD_MODULE_LOAD_PATH};

    /**
     * Instantiation of the resource manager using the abstract base
     * class of a vision-algorithm.
     */
    using Modules = tfv::SharedResource<tfv::Module>;
    Modules modules_;  ///< RAII-style managed vision algorithms.

    std::thread executor_;  ///< Mainloop-Context executing the modules.
    bool active_ = true;    ///< While true, the mainloop is running.
    unsigned execution_latency_ms_ = 100;  ///< Pause during mainloop

    SceneTrees scene_trees_;

    TFV_Callback default_callback_ = nullptr;

    /**
     * Threaded execution context of vision algorithms (modules).
     * This method is started asynchronously during construction of
     * the Api and is running until deconstruction.  It is constantly
     * grabbing frames from the active camera, executing all active
     * modules and activating newly registered modules.
     */
    void execute(void);

    TFV_Result _module_load(std::string const& name, TFV_Int id) {
        Log("API", "ModuleLoad ", name, " ", id);

        if (modules_[id]) {
            return TFV_INVALID_ID;
        }

        auto module = (Module*)(nullptr);
        if (not module_loader_.load_module_from_library(&module, name, id)) {
            Log("API", "Loading library ", name, " failed");
            return module_loader_.last_error();
        }

        if (not camera_control_.acquire()) {
            return TFV_CAMERA_ACQUISITION_FAILED;
        }

        if (not modules_.insert(id, module, [this](Module& module) {

                module_loader_.destroy_module(&module);
            })) {

            camera_control_.release();
            return TFV_MODULE_INITIALIZATION_FAILED;
        }

        module->switch_active(true);
        return TFV_OK;
    }

    void _disable_all_modules(void) {
        modules_.exec_all([this](TFV_Int id, tfv::Module& module) {
            module.disable();
            camera_control_.release();
        });
    }

    void _disable_module_if(
        std::function<bool(tfv::Module const& module)> predicate) {

        modules_.exec_all([this, &predicate](TFV_Int id, tfv::Module& module) {
            if (predicate(module)) {
                module.disable();
                camera_control_.release();
            }
        });
    }

    void _enable_all_modules(void) {
        modules_.exec_all([this](TFV_Int id, tfv::Module& module) {
            if (not module.enabled()) {
                if (camera_control_.acquire()) {
                    module.enable();
                }
            }
        });
    }

    TFV_Result _enable_module(TFV_Int id) {
        return modules_.exec_one(id, [this](tfv::Module& module) {
            if (module.enabled() or camera_control_.acquire()) {
                module.enable();  // possibly redundant
                return TFV_OK;
            } else {
                return TFV_CAMERA_ACQUISITION_FAILED;
            }
        });
    }

    TFV_Result _disable_module(TFV_Int id) {
        return modules_.exec_one(id, [this](tfv::Module& module) {
            module.disable();
            camera_control_.release();
            return TFV_OK;
        });
    }

    bool _scenes_active(void) const { return not scene_trees_.empty(); }

    /// Generate a new module id.
    /// \todo There is currently no code that would prevent regeneration of an
    /// id that is currently in use in case of an overflow and a long running
    /// module.
    TFV_Int _next_public_id(void) const {
        static TFV_Id public_id{0};
        if (++public_id == 0) {
            public_id = 1;
            LogWarning("API", "Overflow of public ids");
        }
        return static_cast<TFV_Int>(public_id);
    }

    TFV_Int _next_internal_id(void) const {
        static TFV_Int internal_id{std::numeric_limits<TFV_Id>::max() + 1};
        return internal_id++;
    }

    TFV_Scene _next_scene_id(void) const {
        static TFV_Scene scene_id{std::numeric_limits<TFV_Id>::max() + 1};
        return scene_id++;
    }
};

Api& get_api(void);
}
