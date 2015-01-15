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
#include "frame.hh"
#include "shared_resource.hh"

#ifdef DEBUG_CAM
#include "window.hh"
#endif  // DEV

namespace tfv {

using FrameWithUserCounter = struct FWUC {
    tfv::Frame the_frame;
    int user;

    FWUC(TFV_Id id, int rows, int columns, int channels, int user)
        : the_frame{id, rows, columns, channels}, user(user) {}

    ~FWUC(void) = default;

    TFV_ImageData* data() const { return the_frame.data; }
    TFV_Int rows() const { return the_frame.rows; }
    TFV_Int columns() const { return the_frame.columns; }
};

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
     * \return True if the api started successfully.
     */
    bool start(void);

    /**
     * Stops all running components.  This is not necessary in general
     * if the Api is being deconstructed in a controlled way. If,
     * however, the client application should crash or exit without stopping
     * all instantiated components, this can be used.
     * \sa start()
     * \return True if the api stopped successfully.
     */
    bool stop(void);

    template <typename Comp, typename... Args>
    TFV_Result component_set(TFV_Id id, TFV_Id camera_id, Args... args) {

        auto result = TFV_INVALID_CONFIGURATION;

        if (tfv::valid<Comp>(args...)) {
            if (components_.managed(id)) {         // reconfiguring requested
                auto component = components_[id];  // ptr

                if (check_type<Comp>(component)) {
                    result = component_reset(component, camera_id, args...);

                } else {
                    result = TFV_INVALID_ID;
                }

            } else {
                result = TFV_CAMERA_ACQUISITION_FAILED;

                if (camera_control_.acquire(camera_id)) {
                    allocate_frame(camera_id);
                    components_.allocate<Comp>(id, camera_id, id, args...);
                    result = TFV_OK;
                }
            }
        }

        return result;
    }

    template <typename Comp, typename... Args>
    TFV_Result component_reset(Comp& component, TFV_Id camera_id,
                               Args... args) {
        auto result = TFV_FEATURE_CONFIGURATION_FAILED;

        if (component->camera_id != camera_id) {  // other cam requested

            result = TFV_CAMERA_ACQUISITION_FAILED;
            if (camera_control_.acquire(camera_id)) {

                allocate_frame(camera_id);
                release_frame(component->camera_id);
                tfv::set<Comp>(static_cast<Comp*>(&component), args...);
                result = TFV_OK;
            }
        } else {

            tfv::set<Comp>(static_cast<Comp*>(&component), args...);
            result = TFV_OK;
        }
        return result;
    }
    template <typename Component>
    TFV_Result component_get(TFV_Id id, TFV_Id& camera_id, TFV_Byte& min_hue,
                             TFV_Byte& max_hue) const {

        auto result = TFV_UNCONFIGURED_ID;

        Component const* ct = nullptr;
        result = get_component<Component>(id, &ct);

        if (ct) {
            tfv::get<Component>(*ct, camera_id, min_hue, max_hue);
        }

        return result;
    }

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

                if (component->active) {
                    result = TFV_OK;
                } else if (camera_control_.acquire(component->camera_id)) {
                    allocate_frame(component->camera_id);
                    component->active = true;
                    result = TFV_OK;
                }
            }
        }
        return result;
    }

    template <typename Component>
    TFV_Result component_stop(TFV_Id id) {
        auto result = TFV_UNCONFIGURED_ID;

        auto component = components_[id];
        if (component) {
            result = TFV_INVALID_ID;

            if (check_type<Component>(component)) {
                auto const camera_id = component->camera_id;
                component->active = false;
                release_frame(camera_id);
                result = TFV_OK;
            }
        }
        return result;
    }

    TFV_String result_string(TFV_Id code) const {
        return result_string_map_[code];
    }

    TFV_Result is_camera_available(TFV_Id camera_id);

    /**
     * Set the time between the execution of active components.
     * This is set to a default of 500ms, meaning that the mainloop
     * pauses for half a second between two executions of the active
     * components. It is recommended to keep it at a decent value
     * because the CPU-load can be quite high with a too low value.
     * \note If no component is active, a minimum latency of 500ms is
     * hardcoded (with the value set here being used if larger).
     * \param ms The duration of the pauses in milliseconds.
     */
    TFV_Result set_execution_latency_ms(TFV_UInt ms) {
        execution_latency_ms_ = std::chrono::milliseconds(ms);
        return TFV_OK;
    }

private:
    CameraControl camera_control_;    ///< Camera access abstraction
    TFVStringMap result_string_map_;  ///< String mapping of Api-return values

    /**
     * Instantiation of the resource manager using the abstract base
     * class of a vision-algorithm.
     */
    using Components = tfv::SharedResource<tfv::Component>;
    Components components_;  ///< RAII-style managed vision algorithms.

    /**
     * Instantiation of the resource manager with the counting camera
     * frame container.
     */
    using Frames = tfv::SharedResource<tfv::FrameWithUserCounter>;
    Frames frames_;  ///< RAII-style managed frames, one per camera.

    std::thread executor_;  ///< Mainloop-Context executing the components.
    bool active_ = true;    ///< While true, the mainloop is running.
    std::chrono::milliseconds execution_latency_ms_{
        500};  ///< Pause between two executions of the mainloop

#ifdef DEBUG_CAM
    Window window;
#endif  // DEV

    /**
     * Threaded execution context of vision algorithms (components).
     * This method is started asynchronously during construction of
     * the Api and is running until deconstruction.  It is constantly
     * grabbing frames from all active cameras, executing all active
     * components and activating newly registered cameras and components.
     */
    void execute(void);

    /**
     * Predicate to check if the argument is of same type as the
     * template parameter.
     * \param[in] component The component to type-check.
     * \return True if the types of the template parameter and argument match.
     */
    template <typename C>
    bool check_type(tfv::Component const* component) const {
        return typeid(*component) == typeid(C);
    }

    /**
     * Predicate to check if the argument is of same type as the
     * template parameter.
     * \param[in] component The component to type-check.
     * \return True if the types of the template parameter and argument match.
     */
    template <typename C>
    bool check_type(tfv::Component const& component) const {
        return typeid(component) == typeid(C);
    }

    /**
     * Helper method for the instantiation of Frames.  A frame has to
     * be allocated only once per camera, after that it can be reused
     * by other components. This method cares for that.
     *
     * \param camera_id The id of the camera to be accessed.
     */
    void allocate_frame(TFV_Id camera_id);

    /**
     * Helper method for the release of Frames.  A frame is allocated
     * only once per camera, after that it is being reused by other
     * components, increasing the usage-counter. This method reduces
     * the count or, once reached 0, issues a release of the frame and
     * the associated camera.
     *
     * \param camera_id The id of the camera to be accessed.
     */
    void release_frame(TFV_Id camera_id);
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
