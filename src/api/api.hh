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

#include "strings.hh"
#include "tinkervision_defines.h"
#include "cameracontrol.hh"
#include "module.hh"
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

    template <typename Comp, typename... Args>
    TFV_Result module_set(TFV_Id id, Args... args) {

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

    template <typename Module>
    TFV_Result module_get(TFV_Id id, TFV_Byte& min_hue,
                          TFV_Byte& max_hue) const {
        auto result = TFV_UNCONFIGURED_ID;
        Module const* ct = nullptr;

        result = get_module<Module>(id, &ct);

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
    TFV_Result module_start(TFV_Id id) {
        auto result = TFV_UNCONFIGURED_ID;

        if (modules_.managed(id)) {
            auto module = modules_[id];
            result = TFV_INVALID_ID;

            if (check_type<Module>(module)) {
                result = TFV_CAMERA_ACQUISITION_FAILED;

                modules_.exec_one(id, [&result, this](tfv::Module& comp) {
                    if (comp.active()) {
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
     * a matter of calling module_start() to resume execution.
     * \note The associated resources (namely the camera handle)
     * will be released to be usable in other contexts, so
     * this might prohibit restart of the module.
     *
     * \param id The id of the module to stop. The type of the associated
     * module has to match Module.
     * \return TFV_OK if the module was stopped; TFV_UNCONFIGURED_ID if
     * the id is not registered; TFV_INVALID_ID if the types don't match.
     */
    template <typename Module>
    TFV_Result module_stop(TFV_Id id) {
        auto result = TFV_UNCONFIGURED_ID;

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
     * Convert Api return code to string.
     * \param code The return code to be represented as string.
     * \return The string representing code.
     */
    TFV_String result_string(TFV_Id code) const {
        return result_string_map_[code];
    }

    /**
     * Check if a camera is available in the system.
     * \result TFV_CAMERA_ACQUISITION_FAILED if the camera is not available,
     * TFV_OK else.
     */
    TFV_Result is_camera_available(void);

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
    CameraControl camera_control_;    ///< Camera access abstraction
    TFVStringMap result_string_map_;  ///< String mapping of Api-return values

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

#ifdef DEBUG_CAM
    Window window;
#endif  // DEV

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
    TFV_Result get_module(TFV_Id id, Module const** module) const {
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
};

Api& get_api(void);
};
