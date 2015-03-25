/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014 philipp.kroos@fh-bielefeld.de

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
#include "component.hh"
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
    bool active_components(void) const { return components_.size(); }

public:
    Api(Api const&) = delete;
    Api& operator=(Api const&) = delete;

    ~Api(void);

    /**
     * Starts execution of all active components.  This is only
     * necessary if the Api had been stopped.  The method is
     * automatically called during construction of the Api.
     * \sa stop()
     * \return TFV_OK if execution started successfully.
     */
    TFV_Result start(void);

    /**
     * Halts (pauses) execution of the main-loop.  This will not do
     * any change to the registered components, only they will stop
     * being executed.  If calling start(), the Api will resume
     * execution with the same configuration.
     * \note Even while the api is paused there can still be new components
     * registered. They will start execution once start() is called.
     * \sa quit() If the whole thing shall be stopped.
     * \return A result code:
     *     - TFV_OK when execution halted successfully.
     *     - TFV_EXEC_THREAD_FAILURE when the thread is still running.
     */
    TFV_Result stop(void);

    /**
     * Stops all running components.  This is not necessary in general
     * if the Api is being deconstructed in a controlled way. If,
     * however, the client application should crash or exit without stopping
     * all instantiated components, this can be used.
     * \sa start()
     * \return A result code:
     *     - TFV_OK when execution halted successfully.
     *     - TFV_EXEC_THREAD_FAILURE when the thread is still running.
     */
    TFV_Result quit(void);

    template <typename Comp, typename... Args>
    TFV_Result component_set(TFV_Id id, Args... args) {

        auto result = TFV_INVALID_CONFIGURATION;

        if (tfv::valid<Comp>(args...)) {
            if (components_.managed(id)) {         // reconfiguring requested
                auto component = components_[id];  // ptr

                result = TFV_INVALID_ID;

                if (check_type<Comp>(component)) {
                    tfv::set<Comp>(static_cast<Comp*>(component), args...);
                    result = TFV_OK;
                }

            } else {  // new component
                result = TFV_CAMERA_ACQUISITION_FAILED;

                if (camera_control_.acquire()) {
                    components_.allocate<Comp>(id, args...);
                    result = TFV_OK;
                }
            }
        }

        return result;
    }

    template <typename Component>
    TFV_Result component_get(TFV_Id id, TFV_Byte& min_hue,
                             TFV_Byte& max_hue) const {
        auto result = TFV_UNCONFIGURED_ID;
        Component const* ct = nullptr;

        result = get_component<Component>(id, &ct);

        if (ct) {
            tfv::get<Component>(*ct, min_hue, max_hue);
        }

        return result;
    }

    /**
     * Start a component which was already initialized by
     * component_set().  This method succeeds if the component was
     * already started or can be started.  This in turn is only
     * possible if a component is registered under the given id, that
     * component is of the type of the template parameter, and the
     * camera set in the component can be acquired.
     *
     * \param[in] id The id of the component to start.
     *
     * \return TFV_UNCONFIGURED_ID if no component is registered with
     * the given id; TFV_INVALID_ID if the registered component is not
     * of the correct type; TFV_CAMERA_ACQUISATION_FAILED if the
     * camera specified for the component is not available; TFV_OK iff
     * the component is running after returning.
     */
    template <typename Component>
    TFV_Result component_start(TFV_Id id) {
        auto result = TFV_UNCONFIGURED_ID;

        if (components_.managed(id)) {
            auto component = components_[id];
            result = TFV_INVALID_ID;

            if (check_type<Component>(component)) {
                result = TFV_CAMERA_ACQUISITION_FAILED;

                components_.exec_one(id, [&result, this](tfv::Component& comp) {
                    if (comp.active) {
                        result = TFV_OK;
                    } else if (camera_control_.acquire()) {
                        comp.active = true;
                        result = TFV_OK;
                    }
                });
            }
        }

        return result;
    }

    /**
     * Pause a component. This will not remove the component but rather
     * prevent it from being executed. The id is still reserved and it's
     * a matter of calling component_start() to resume execution.
     * \note The associated resources (namely the camera handle)
     * will be released to be usable in other contexts, so
     * this might prohibit restart of the component.
     *
     * \param id The id of the component to stop. The type of the associated
     * component has to match Component.
     * \return TFV_OK if the component was stopped; TFV_UNCONFIGURED_ID if
     * the id is not registered; TFV_INVALID_ID if the types don't match.
     */
    template <typename Component>
    TFV_Result component_stop(TFV_Id id) {
        auto result = TFV_UNCONFIGURED_ID;

        auto component = components_[id];
        if (component) {
            result = TFV_INVALID_ID;

            if (check_type<Component>(component)) {
                components_.exec_one(id, [this](tfv::Component& comp) {
                    comp.active = false;
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
     * Set the time between the execution of active components.
     * This is set to a default of 100ms, meaning that the mainloop
     * pauses for half a second between two executions of the active
     * components. It is recommended to keep it at a decent value
     * because the CPU-load can be quite high with a too low value.
     * However, with a lot of active components, this will reduce
     * respondability.
     * \note If no component is active, a minimum latency of 200ms is
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
    using Components = tfv::SharedResource<tfv::Component>;
    Components components_;  ///< RAII-style managed vision algorithms.

    std::thread executor_;  ///< Mainloop-Context executing the components.
    bool active_ = true;    ///< While true, the mainloop is running.
    unsigned execution_latency_ms_ = 100;  ///< Pause during mainloop

#ifdef DEBUG_CAM
    Window window;
#endif  // DEV

    /**
     * Threaded execution context of vision algorithms (components).
     * This method is started asynchronously during construction of
     * the Api and is running until deconstruction.  It is constantly
     * grabbing frames from the active camera, executing all active
     * components and activating newly registered components.
     */
    void execute(void);

    /**
     * Predicate to check if the argument is of same type as the
     * template parameter.
     * \param[in] component The component to type-check.
     * \return True if the types of the template parameter and argument
     * match.
     */
    template <typename C>
    bool check_type(tfv::Component const* component) const {
        return typeid(*component) == typeid(C);
    }

    /**
     * Predicate to check if the argument is of same type as the
     * template parameter.
     * \param[in] component The component to type-check.
     * \return True if the types of the template parameter and argument
     * match.
     */
    template <typename C>
    bool check_type(tfv::Component const& component) const {
        return typeid(component) == typeid(C);
    }

    /**
     * Start the default camera or increase the usagecounter.
     *
     * \return True if the camera could be acquired.
     */
    bool start_camera(void);

    template <typename Component>
    TFV_Result get_component(TFV_Id id, Component const** component) const {
        auto result = TFV_UNCONFIGURED_ID;

        if (components_.managed(id)) {
            result = TFV_INVALID_ID;
            auto const& component_ = components_[id];

            if (check_type<Component>(component_)) {
                result = TFV_OK;
                *component = static_cast<Component const*>(&component_);
            }
        }

        return result;
    }
};

/*
    struct ApiRunner {
        void operator()(Api* api) {
            auto timeout = std::chrono::seconds(5);
            auto checkpoint = std::chrono::system_clock::now();
            auto now = std::chrono::system_clock::now();

            while (true) {
                now = std::chrono::system_clock::now();
                if (api->active_components()) {
                    checkpoint = now;
                }
                else {
                    std::cout << "No active components" << std::endl;
                    if ((now - checkpoint) > timeout) {
                        std::cout << "Shutting down" << std::endl;
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            api->stop();
            delete api;
            api = nullptr;
        }
    };
*/
Api& get_api(void);
};
