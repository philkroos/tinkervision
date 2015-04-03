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

#include "strings.hh"
#include "tinkervision_defines.h"
#include "cameracontrol.hh"
#include "executable.hh"
#include "dummy.hh"
#include "image.hh"

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
     * to be referenced later, i.e. sth. like a single-shot module (e.g. module
     * Snapshot).  The module will receive an internally generated unique id
     * which can not conflict with the ids assignable by the user.
     * \return
     * - TFV_CAMERA_ACQUISITION_FAILED: if the camera is not available
     * - TFV_MODULE_INITIALIZATION_FAILED: if an error occurs during allocation
     *   of the module
     * - TFV_OK: this should be expect.
     */
    template <typename Comp>
    TFV_Result module_set(void) {
        return _module_set<Comp>(_next_internal_id());
    }

    TFV_Result start_idle(void) {
        auto result = TFV_OK;  // optimistic because startable only once

        if (not idle_process_running_) {
            result = module_set<Dummy>();
        }
        idle_process_running_ = (result == TFV_OK);
        return result;
    }

    /**
     * Insert and activate or reconfigure a module.
     * \parm[in] id The unique id of the module under which it may be identified
     *   (i.e. in future calls to get/set/free...)
     * \parm[in] ...args The module dependent list of constructor arguments
     * \return
     * - TFV_INVALID_CONFIGURATION: if the arguments can not be used to
     *   construct a valid module of type Comp
     * - TFV_INVALID_ID: if a module with the given id already exists but is not
     *   of type Comp
     * - TFV_CAMERA_ACQUISITION_FAILED: if a new module shall be constructed but
     *   the camera is not available
     * - TFV_MODULE_INITIALIZATION_FAILED: if an error occurs during allocation
     *   of the module
     * - TFV_OK: this should be expect.
     */
    template <typename Comp, typename... Args>
    TFV_Result module_set(TFV_Id id, Args... args) {
        return _module_set<Comp>(static_cast<TFV_Int>(id), args...);
    }

    template <typename Module>
    TFV_Result module_get(TFV_Id id, TFV_Byte& min_hue,
                          TFV_Byte& max_hue) const {
        auto result = TFV_UNCONFIGURED_ID;
        Module const* ct = nullptr;

        result = _get_module<Module>(static_cast<TFV_Int>(id), &ct);

        if (ct) {
            tfv::get<Module>(*ct, min_hue, max_hue);
        }

        return result;
    }

    /**
     * Start a module which was already initialized by
     * module_set().  This method succeeds if the module was
     * already started or can be started.  This in turn is only
     * possible if a module is registered under the given id, that
     * module is of the type of the template parameter, and the
     * camera set in the module can be acquired.
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

            if (check_type<Module>(module)) {
                result = TFV_CAMERA_ACQUISITION_FAILED;

                modules_.exec_one(id, [&result, this](tfv::Module& comp) {
                    if (comp.is_active()) {
                        result = TFV_OK;
                    } else if (camera_control_.acquire()) {
                        comp.activate();
                        result = TFV_OK;
                    }
                });
            }
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
     * this might prohibit restart of the module. If however the camera is used
     * by other modules as well, it will stay open.
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
        auto result = TFV_UNCONFIGURED_ID;
        auto id = static_cast<TFV_Int>(module_id);

        auto module = modules_[id];
        if (module) {
            result = TFV_INVALID_ID;

            if (check_type<Module>(module)) {
                modules_.exec_one(id, [this](tfv::Module& comp) {
                    comp.deactivate();
                    camera_control_.release();
                });
                result = TFV_OK;
            }
        }
        return result;
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
        auto result = TFV_UNCONFIGURED_ID;
        auto id = static_cast<TFV_Int>(module_id);

        auto module = modules_[id];
        if (module) {
            result = TFV_INVALID_ID;

            if (check_type<Module>(module)) {
                modules_.exec_one(id, [this](tfv::Module& comp) {
                    comp.deactivate();
                    comp.mark_for_removal();
                    camera_control_.release();
                });
                result = TFV_OK;
            }
        }
        return result;
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
     * Retrieve the frame settings from the camera. This can only work if the
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

private:
    CameraControl camera_control_;      ///< Camera access abstraction
    TFVStringMap result_string_map_;    ///< String mapping of Api-return values
    bool idle_process_running_{false};  ///< Dummy module activated?

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
    bool check_type(tfv::Module const* module) const {
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
    bool check_type(tfv::Module const& module) const {
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

            if (check_type<Module>(module_)) {
                result = TFV_OK;
                *module = static_cast<Module const*>(&module_);
            }
        }

        return result;
    }

    template <typename Comp, typename... Args>
    TFV_Result _module_set(TFV_Int id, Args... args) {

        auto result = TFV_INVALID_CONFIGURATION;

        if (tfv::valid<Comp>(args...)) {
            if (modules_.managed(id)) {      // reconfiguring requested
                auto module = modules_[id];  // ptr

                result = TFV_INVALID_ID;

                if (check_type<Comp>(module)) {
                    tfv::set<Comp>(static_cast<Comp*>(module), args...);
                    result = TFV_OK;
                }

            } else {  // new module
                result = TFV_CAMERA_ACQUISITION_FAILED;

                if (camera_control_.acquire()) {
                    result = TFV_MODULE_INITIALIZATION_FAILED;

                    if (modules_.allocate<Comp>(id, args...)) {
                        result = TFV_OK;
                    } else {
                        camera_control_.release();
                    }
                }
            }
        }

        return result;
    }

    TFV_Int _next_internal_id(void) const {
        static TFV_Int internal_id{std::numeric_limits<TFV_Id>::max() + 1};
        return internal_id++;
    }
};

Api& get_api(void);
}
