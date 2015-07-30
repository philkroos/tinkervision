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
#include <atomic>

#include "strings.hh"
#include "tinkervision_defines.h"
#include "cameracontrol.hh"
#include "dummy.hh"
#include "image.hh"
#include "scenetrees.hh"
#include "logger.hh"

#include "shared_resource.hh"

#if defined DEV or defined DEBUG_CAM
#include <iostream>
#include "window.hh"
#endif

namespace tfv {

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
     * \return TFV_OK if execution started successfully.
     */
    TFV_Result start(void);

    /**
     * Halts (pauses) execution of the main-loop.  This will not do
     * any change to the registered modules, only they will stop
     * being executed.  If calling start(), the Api will resume
     * execution with the same configuration.
     * \note Even while the api is paused there can still be new modules
     * registered. They will start execution once start() is called.
     * \sa quit() If the whole thing shall be stopped.
     * \return A result code:
     *     - TFV_OK when execution halted successfully.
     *     - TFV_EXEC_THREAD_FAILURE when the thread is still running.
     */
    TFV_Result stop(void);

    /**
     * Stops all running modules.  This is not necessary in general
     * if the Api is being deconstructed in a controlled way. If,
     * however, the client application should crash or exit without stopping
     * all instantiated modules, this can be used.
     * \sa start()
     * \return A result code:
     *     - TFV_OK when execution halted successfully.
     *     - TFV_EXEC_THREAD_FAILURE when the thread is still running.
     */
    TFV_Result quit(void);

    /**
     * Insert and activate a module.
     * Use this to instantiate a module without parameters which does not have
     * to be referenced later and is a single-shot module (e.g. module
     * Snapshot).  The module will receive an internally generated unique id
     * which can not conflict with the ids assignable by the user.
     * \return
     * - TFV_CAMERA_ACQUISITION_FAILED: if the camera is not available
     * - TFV_MODULE_INITIALIZATION_FAILED: if an error occurs during allocation
     *   of the module
     * - TFV_OK: this should be expect.
     */
    template <typename Comp>
    TFV_Result module_once(void) {
        return _module_set<Comp>(_next_internal_id(),
                                 Module::Tag::ExecAndRemove);
    }

    /**
     * Preselect the framesize. This can only be successfull if the camera is
     * not active currently, i.e. if the API is in a stopped state and no module
     * is active.
     * \return
     * - TFV_CAMERA_SETTINGS_FAILED if there are active modules already
     * - TFV_OK else
     */
    TFV_Result preselect_framesize(TFV_Size width, TFV_Size height) {
        return camera_control_.preselect_framesize(width, height)
                   ? TFV_OK
                   : TFV_CAMERA_SETTINGS_FAILED;
    }

    /**
     * Start an idle process, i.e. a module which will never be executed.
     * This
     * is a lightweight module which will not trigger frame grabbing.
     * However, once started, it will keep the camera device blocked
     * so it may be used to hold on the camera handle even if no 'real'
     * module is running.  This dummy process can not be referred to
     * since the assigned id is not retreivable by the user,
     * and it is (currently) not deactivatable unless \code quit()
     * is called.  Also, the process will only be started once, no
     * matter how often this method gets called.
     * \return TFV_OK if the process is running afterwards
     */
    TFV_Result start_idle(void) {
        auto result = TFV_OK;  // optimistic because startable only once

        if (not idle_process_running_) {
            result = module_set<Dummy>(_next_internal_id());
        }
        idle_process_running_ = (result == TFV_OK);
        return result;
    }

    /**
     * Insert and activate or reconfigure a module.
     * \param[in] id The unique id of the module under which it may be
     * identified (i.e. in future calls to get/set/free...)
     * \param[in] ...args The module dependent list of constructor arguments
     * \return
     * - TFV_INVALID_CONFIGURATION: if the arguments can not be used to
     *   construct a valid module of type Comp
     * - TFV_INVALID_ID: if a module with the given id already exists but is
     * not of type Comp
     * - TFV_CAMERA_ACQUISITION_FAILED: if a new module shall be constructed
     * but the camera is not available
     * - TFV_MODULE_INITIALIZATION_FAILED: if an error occurs during
     * allocation of the module
     * - TFV_OK: this should be expected.
     */
    template <typename Comp, typename... Args>
    TFV_Result module_set(TFV_Id id, Args... args) {
        return _module_set<Comp>(static_cast<TFV_Int>(id), Module::Tag::None,
                                 args...);
    }

    template <typename Module>
    TFV_Result module_get(TFV_Id id, TFV_Byte& min_hue,
                          TFV_Byte& max_hue) const {
        auto result = TFV_UNCONFIGURED_ID;
        Module const* module = nullptr;

        result = _get_module<Module>(static_cast<TFV_Int>(id), &module);

        if (module) {
            tfv::get<Module>(*module, min_hue, max_hue);
        }

        return result;
    }

    /**
     * Start a module which was already initialized by
     * module_set().  This method succeeds if the module was
     * already started or can be started.  This in turn is only
     * possible if a module is registered under the given id, that
     * module is of the type of the template parameter, and the
     * camera can be acquired.
     *
     * \param[in] id The id of the module to start.
     *
     * \return TFV_UNCONFIGURED_ID if no module is registered with
     * the given id; TFV_INVALID_ID if the registered module is not
     * of the correct type; TFV_CAMERA_ACQUISATION_FAILED if the
     * camera specified for the module is not available; TFV_OK iff
     * the module is running after returning.
     */
    template <typename Module>
    TFV_Result module_start(TFV_Id module_id) {
        auto result = TFV_UNCONFIGURED_ID;
        auto id = static_cast<TFV_Int>(module_id);

        if (modules_.managed(id)) {
            auto module = modules_[id];
            result = TFV_INVALID_ID;

            if (_check_type<Module>(module)) {
                result = _enable_module(module_id);
            }
        }

        return result;
    }

    /**
     * Start a module which was already initialized by
     * module_set().  This method succeeds if the module was
     * already started or can be started.  This in turn is only
     * possible if a module is registered under the given id
     * and the camera can be acquired.
     *
     * \param[in] id The id of the module to start.
     *
     * \return TFV_UNCONFIGURED_ID if no module is registered with
     * the given id; TFV_INVALID_ID if the registered module is not
     * of the correct type; TFV_CAMERA_ACQUISATION_FAILED if the
     * camera specified for the module is not available; TFV_OK iff
     * the module is running after returning.
     */
    TFV_Result start_id(TFV_Id module_id) {
        auto result = TFV_UNCONFIGURED_ID;
        auto id = static_cast<TFV_Int>(module_id);

        if (modules_.managed(id)) {
            result = _enable_module(module_id);
        }

        return result;
    }

    /**
     * Pause a module. This will not remove the module but rather
     * prevent it from being executed. The id is still reserved and it's
     * a matter of calling module_start() to resume execution.  To actually
     * remove the module, call module_remove.
     * \note The associated resources (namely the camera handle)
     * will be released (once) to be usable in other contexts, so
     * this might prohibit restart of the module. If however the camera is
     * used by other modules as well, it will stay open.
     *
     * \param id The id of the module to stop. The type of the associated
     * module has to match Module.
     * \return
     *  - TFV_OK if the module was stopped and marked for removal
     *  - TFV_UNCONFIGURED_ID if the id is not registered
     *  - TFV_INVALID_ID if the types don't match
     */
    template <typename Module>
    TFV_Result module_stop(TFV_Id module_id) {
        auto id = static_cast<TFV_Int>(module_id);

        auto module = modules_[id];
        if (module == nullptr) {
            return TFV_UNCONFIGURED_ID;
        }

        if (not _check_type<Module>(module)) {
            return TFV_INVALID_ID;
        }

        return _disable_module(id);
    }

    /**
     * Pause a module. This will not remove the module but rather
     * prevent it from being executed. The id is still reserved and it's
     * a matter of calling start_id() to resume execution.  To actually
     * remove the module, call remove_id().
     * \note The associated resources (namely the camera handle)
     * will be released (once) to be usable in other contexts, so
     * this might prohibit restart of the module. If however the camera is
     * used by other modules as well, it will stay open.
     *
     * \param id The id of the module to stop. The type of the associated
     * module has to match Module.
     * \return
     *  - TFV_OK if the module was stopped and marked for removal
     *  - TFV_UNCONFIGURED_ID if the id is not registered
     */
    TFV_Result stop_id(TFV_Id module_id) {
        auto id = static_cast<TFV_Int>(module_id);

        if (not modules_.managed(id)) {
            return TFV_UNCONFIGURED_ID;
        }

        return _disable_module(module_id);
    }

    /**
     * Stop and remove a module.  After this, the id is no longer valid.
     * \note The associated resources (namely the camera handle)
     * will be released (once) to be usable in other contexts. This might
     * free the actual device if it is not used by another module.
     *
     * \param id The id of the module to stop. The type of the associated
     * module has to match Module.
     * \return
     *  - TFV_OK if the module was stopped and marked for removal
     *  - TFV_UNCONFIGURED_ID if the id is not registered
     *  - TFV_INVALID_ID if the types don't match
     */
    template <typename Module>
    TFV_Result module_remove(TFV_Id module_id) {
        auto id = static_cast<TFV_Int>(module_id);

        auto module = modules_[id];
        if (not modules_.managed(id)) {
            return TFV_UNCONFIGURED_ID;
        }

        if (not _check_type<Module>(module)) {
            return TFV_INVALID_ID;
        }

        if (_scenes_active()) {
            return TFV_NOT_IMPLEMENTED;
        }

        return modules_.exec_one(id, [this](tfv::Module& comp) {
            comp.disable();
            comp.tag(Module::Tag::Removable);
            camera_control_.release();
            return TFV_OK;
        });
    }

    /**
     * Stop and remove a module.  After this, the id is no longer valid.
     * \note The associated resources (namely the camera handle)
     * will be released (once) to be usable in other contexts. This might
     * free the actual device if it is not used by another module.
     *
     * \param id The id of the module to stop. The type of the associated
     * module has to match Module.
     * \return
     *  - TFV_OK if the module was stopped and marked for removal
     *  - TFV_UNCONFIGURED_ID if the id is not registered
     */
    TFV_Result remove_id(TFV_Id module_id) {
        auto id = static_cast<TFV_Int>(module_id);

        if (not modules_[id]) {
            return TFV_UNCONFIGURED_ID;
        }

        return modules_.exec_one(id, [this](tfv::Module& comp) {
            comp.disable();
            comp.tag(Module::Tag::Removable);
            camera_control_.release();
            return TFV_OK;
        });
    }

    /**
     * Convert Api return code to string.
     * \param[in] code The return code to be represented as string.
     * \return The string representing code
     */
    TFV_String result_string(TFV_Id code) const {
        return result_string_map_[code];
    }

    /**
     * Check if a camera is available in the system.
     * \return
     *  - TFV_CAMERA_ACQUISITION_FAILED if the camera is not available,
     *  - TFV_OK else
     */
    TFV_Result is_camera_available(void) {
        return camera_control_.is_available() ? TFV_OK
                                              : TFV_CAMERA_ACQUISITION_FAILED;
    }

    /**
     * Retrieve the frame settings from the camera. This can only work if
     * the
     * camera was opened already
     * \param[out] width The framewidth in pixels
     * \param[out] width The frameheight in pixels
     * \return
     *  - TFV_CAMERA_NOT_AVAILABLE if the camera is not open
     *  - TFV_OK else.
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
        execution_latency_ms_ = ms;
        return TFV_OK;
    }

    TFV_Result chain(TFV_Id first, TFV_Id second);

    /**
     * Start a scene which is a directed chain of modules.
     *
     * \note Replaces the broken chain-functionality
     * If a scene is started:
     * - All modules not part of a scene are deactivated
     */
    TFV_Result scene_start(TFV_Id module_id, TFV_Scene* scene_id) {
        Log("API", "Starting scene");

        if (not modules_.managed(module_id)) {
            return TFV_INVALID_ID;
        }

        if (modules_[module_id]->tags() & Module::Tag::ExecAndRemove) {
            return TFV_NOT_IMPLEMENTED;
        }

        // \todo This would alter the state of the system if the
        // requested operation fails, which is not expected behaviour.
        if (scene_trees_.empty()) {
            _disable_all_modules();
        }

        auto result = _enable_module(module_id);
        if (result != TFV_OK) {
            return result;
        }

        return scene_trees_.scene_start(_next_scene_id(), module_id);
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

private:
    CameraControl camera_control_;      ///< Camera access abstraction
    TFVStringMap result_string_map_;    ///< String mapping of Api-return values
    bool idle_process_running_{false};  ///< Dummy module activated?
    std::atomic_char chained_{0};       ///< Sequential execution?

    Image image_;  ///< The default image

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

    /**
     * Threaded execution context of vision algorithms (modules).
     * This method is started asynchronously during construction of
     * the Api and is running until deconstruction.  It is constantly
     * grabbing frames from the active camera, executing all active
     * modules and activating newly registered modules.
     */
    void execute(void);

    /**
     * Predicate to check if the argument is of same type as the
     * template parameter.
     * \param[in] module The module to type-check.
     * \return True if the types of the template parameter and argument
     * match.
     */
    template <typename C>
    bool _check_type(tfv::Module const* module) const {
        return typeid(*module) == typeid(C);
    }

    /**
     * Predicate to check if the argument is of same type as the
     * template parameter.
     * \param[in] module The module to type-check.
     * \return True if the types of the template parameter and argument
     * match.
     */
    template <typename C>
    bool _check_type(tfv::Module const& module) const {
        return typeid(module) == typeid(C);
    }

    /**
     * Start the default camera or increase the usagecounter.
     *
     * \return True if the camera could be acquired.
     */
    bool start_camera(void);

    template <typename Module>
    TFV_Result _get_module(TFV_Int id, Module const** module) const {
        auto result = TFV_UNCONFIGURED_ID;

        if (modules_.managed(id)) {
            result = TFV_INVALID_ID;
            auto const& module_ = modules_[id];

            if (_check_type<Module>(module_)) {
                result = TFV_OK;
                *module = static_cast<Module const*>(&module_);
            }
        }

        return result;
    }

    template <typename Comp, typename... Args>
    TFV_Result _module_set(TFV_Int id, Module::Tag tags, Args... args) {

        if (not tfv::valid<Comp>(args...)) {
            return TFV_INVALID_CONFIGURATION;
        }

        if (not modules_.managed(id)) {
            return _module_set_new<Comp>(id, tags, args...);
        }

        // reconfiguration requested

        auto module = modules_[id];  // ptr

        if (not _check_type<Comp>(module)) {
            return TFV_INVALID_ID;
        }

        // GCC4.7 does not support binding of ...args to lambdas
        // yet, this is the workaround to get the arguments into
        // the execution context (which expects a 1-parameter func)
        auto two_parameter = [this](tfv::Module& module, Args... args) {
            tfv::set<Comp>(static_cast<Comp*>(&module), args...);
            if (module.enabled() or camera_control_.acquire()) {
                module.enable();  // redundant
                return TFV_OK;
            } else {
                return TFV_CAMERA_ACQUISITION_FAILED;
            }
        };
        auto one_parameter =  // currying
            std::bind(two_parameter, std::placeholders::_1, args...);

        return modules_.exec_one(id, one_parameter);
    }

    template <typename Comp, typename... Args>
    TFV_Result _module_set_new(TFV_Int id, Module::Tag tags, Args... args) {

        if (not camera_control_.acquire()) {
            return TFV_CAMERA_ACQUISITION_FAILED;
        }
        if (modules_.allocate<Comp>(id, nullptr, tags, args...)) {
            camera_control_.release();
            return TFV_MODULE_INITIALIZATION_FAILED;
        }

        return TFV_OK;
    }

    void _disable_all_modules(void) {
        modules_.exec_all([this](TFV_Int id, tfv::Module& module) {
            module.disable();
            camera_control_.release();
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
        return modules_.exec_one(id, [this](tfv::Module& comp) {
            if (comp.enabled() or camera_control_.acquire()) {
                comp.enable();  // possibly redundant
                return TFV_OK;
            } else {
                return TFV_CAMERA_ACQUISITION_FAILED;
            }
        });
    }

    TFV_Result _disable_module(TFV_Int id) {
        return modules_.exec_one(id, [this](tfv::Module& comp) {
            comp.disable();
            camera_control_.release();
            return TFV_OK;
        });
    }

    bool _scenes_active(void) const { return not scene_trees_.empty(); }

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
