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

#include <chrono>
#include <iostream>
#include <thread>
#include <typeinfo>

#ifdef DEV
#include <opencv2/opencv.hpp>
#endif  // DEV

#include "api.hh"
#include "colortracking.hh"
#include "component.hh"

tfv::Api::Api(void) { (void)start(); }

tfv::Api::~Api(void) { stop(); }

bool tfv::Api::start(void) {
    if (not executor_.joinable()) {
        executor_ = std::thread(&tfv::Api::execute, this);
    }
    return executor_.joinable();
}

bool tfv::Api::stop(void) {
    if (executor_.joinable()) {
        active_ = false;
        executor_.join();
    }
    return not executor_.joinable();
}

void tfv::Api::execute(void) {
#ifdef DEBUG_CAM
    auto update_frames = [this](TFV_Id id, tfv::FrameWithUserCounter& frame) {
        auto grabbed = camera_control_.get_frame(id, frame.data());
        if (grabbed) {
            window.update(id, frame.data(), frame.rows(), frame.columns());
        } else {
            std::cout << "Could not grab frame" << std::endl;
        }
    };

#else
    // Refresh each camera
    auto update_frames = [this](TFV_Id id, tfv::FrameWithUserCounter& frame) {
        camera_control_.get_frame(id, frame.data());
    };
#endif

    // Execute each active component
    auto const& frames = const_cast<Frames const&>(frames_);
    auto update_components = [this, &frames](TFV_Id id,
                                             tfv::Component& component) {
        if (component.active) {
            auto const& cam = component.camera_id;
            if (frames.managed(cam)) {

                auto const& frame = frames[cam];
                component.execute(frame.data(), frame.rows(), frame.columns());
            }
        }
    };

    // mainloop
    while (active_) {
        frames_.exec_all(update_frames);
        components_.exec_all(update_components);
    }
}

tfv::Api& tfv::get_api(void) {
    static Api* api = nullptr;

    // It would be easier to just return a static
    // Api-instance from get_api.
    // However, since the library is supposed to be run in
    // the context of a
    // damon, the calling context potentially does not
    // finish for a long time.
    // Therefore, watching the Api-usage with the following
    // static inner context
    // makes it possible to release the Api-ressource after
    // some 'timeout'.
    // For the user, this is transparent since the Api will
    // only be released if
    // it is inactive, in which case reinstantiating the Api
    // will have the same
    // result as would have accessing a 'sleeping' one.
    static auto exec = [](tfv::Api* api) {
        auto timeout = std::chrono::seconds(60);
        auto checkpoint = std::chrono::system_clock::now();
        while (true) {
            auto now = std::chrono::system_clock::now();
            if (api->active_components()) {
                checkpoint = now;
            } else {
                // Todo: Share some execution context to
                // prevent the rare
                // case of conflict with a get_api call;
                // e.g. update the
                // checkpoint from the outer context and
                // lock it with a mutex.
                std::cout << "No active components" << std::endl;
                if ((now - checkpoint) > timeout) {
                    std::cout << "Shutting down" << std::endl;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        if (api) {
            delete api;
        }
        api = nullptr;
    };

    static std::thread api_runner;
    if (not api) {
        api = new Api;
        api_runner = std::thread(exec, api);
        api_runner.detach();
    }

    return *api;
}

TFV_Result tfv::Api::colortracking_set(TFV_Id id, TFV_Id camera_id,
                                       TFV_Byte min_hue, TFV_Byte max_hue,
                                       TFV_CallbackColortrack callback,
                                       TFV_Context context) {

    return component_set<tfv::Colortracking>(id, camera_id, min_hue, max_hue,
                                             callback, context);
}

TFV_Result tfv::Api::colortracking_get(TFV_Id id, TFV_Id& camera_id,
                                       TFV_Byte& min_hue,
                                       TFV_Byte& max_hue) const {

    auto result = TFV_UNCONFIGURED_ID;

    tfv::Colortracking const* ct = nullptr;
    result = component_get<tfv::Colortracking>(id, &ct);

    if (ct) {
        tfv::get<tfv::Colortracking>(*ct, camera_id, min_hue, max_hue);
    }

    return result;
}

TFV_Result tfv::Api::colortracking_stop(TFV_Id id) {
    return component_stop<tfv::Colortracking>(id);
}

TFV_Result tfv::Api::colortracking_start(TFV_Id id) {
    return component_start<tfv::Colortracking>(id);
}

TFV_Result tfv::Api::is_camera_available(TFV_Id camera_id) {
    auto result = TFV_CAMERA_ACQUISITION_FAILED;
    if (camera_control_.is_available(camera_id)) {
        result = TFV_OK;
    }

    return result;
}

template <typename Comp, typename... Args>
TFV_Result tfv::Api::component_set(TFV_Id id, TFV_Id camera_id, Args... args) {

    auto result = TFV_INVALID_CONFIGURATION;

    if (tfv::valid<Comp>(args...)) {
        if (components_.managed(id)) {  // reconfiguring requested

            auto component = components_[id];
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
TFV_Result tfv::Api::component_reset(Comp& component, TFV_Id camera_id,
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

void tfv::Api::allocate_frame(TFV_Id camera_id) {
    if (not frames_.managed(camera_id)) {
        int rows, columns, channels = 0;
        camera_control_.get_properties(camera_id, rows, columns, channels);
        auto const user = 1;
        frames_.allocate(camera_id, camera_id, rows, columns, channels, user);
    } else {
        frames_[camera_id]->user++;
    }
}

void tfv::Api::release_frame(TFV_Id camera_id) {
    if (frames_.managed(camera_id)) {

        frames_[camera_id]->user--;
        if (not frames_[camera_id]->user) {

            frames_.free(camera_id);
            camera_control_.release(camera_id);
        }
    }
}

template <typename Component>
TFV_Result tfv::Api::component_get(TFV_Id id,
                                   Component const** component) const {
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

template <typename Component>
TFV_Result tfv::Api::component_start(TFV_Id id) {
    auto result = TFV_UNCONFIGURED_ID;

    if (components_.managed(id)) {

        auto component = components_[id];
        result = TFV_INVALID_ID;

        if (check_type<Component>(component)) {
            result = TFV_CAMERA_ACQUISITION_FAILED;

            if (camera_control_.acquire(component->camera_id)) {

                component->active = true;
                result = TFV_OK;
            }
        }
    }
    return result;
}

template <typename Component>
TFV_Result tfv::Api::component_stop(TFV_Id id) {
    auto result = TFV_UNCONFIGURED_ID;

    if (components_.managed(id)) {

        auto component = components_[id];
        auto const camera_id = component->camera_id;
        component->active = false;
        components_.free(id);
        release_frame(camera_id);
        result = TFV_OK;
    }

    return result;
}
