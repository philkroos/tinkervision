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

#include <limits>
#include <chrono>
#include <iostream>
#include <thread>
#include <future>
#include <typeinfo>

#ifdef DEV
#include <opencv2/opencv.hpp>
#endif  // DEV

#include "api.hh"
#include "colortracking.hh"

tfv::Api::Api(void) : camera_control_{TFV_MAX_USERS_PER_CAM} { (void)start(); }

tfv::Api::~Api(void) {

    {
        std::lock_guard<std::mutex> lock(frame_lock_);

        for (auto& frame : frames_) {
            if (frame.second) {
                delete frame.second;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(components_lock_);

        for (auto& component : components_) {
            if (component.second) {
                delete component.second;
            }
        }
    }

    stop();
}

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
    while (active_) {
        // if (colortracker_.active()) {
        // refresh frames
        //        std::cout << "For " << frames_.size() << " frames" <<
        // std::endl;
        {
            std::lock_guard<std::mutex> lock(frame_lock_);
            for (auto& frame : frames_) {
#ifdef DEV
                auto grabbed =
                    camera_control_.get_frame(frame.first, frame.second->data);
                // std::cout << "Grabbed frame from cam " << frame.first <<
                // std::endl;
                if (grabbed and frame.first == 0) {  // show cam0
                    window.update(frame.second->data, frame.second->rows,
                                  frame.second->columns);
                }
#else
                (void)camera_control_.get_frame(frame.first,
                                                frame.second->data);
#endif
            }

            {
                // Todo: make this lock specific; i.e. add a
                // smart-pointer-like component-wrapper providing locking
                std::lock_guard<std::mutex> lock(components_lock_);
                for (auto& component : components_) {
                    if (component.second->active) {
                        auto frame = frames_[component.second->camera_id];
                        // std::cout << "Passing frame " <<
                        // component.second->camera_id()
                        //           << std::endl;
                        component.second->execute(frame->data, frame->rows,
                                                  frame->columns);
                        //        } else {
                        // no work currently, wait a sec
                        //            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        //        }
                    }
                }
            }
        }
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
                                       TFV_Callback callback,
                                       TFV_Context context) {

    return component_set<tinkervision::Colortracking>(
        id, camera_id, min_hue, max_hue, callback, context);
}

TFV_Result tfv::Api::colortracking_get(TFV_Id id, TFV_Id& camera_id,
                                       TFV_Byte& min_hue,
                                       TFV_Byte& max_hue) const {

    auto result = TFV_UNCONFIGURED_ID;

    tinkervision::Colortracking* ct = nullptr;
    result = component_get<tinkervision::Colortracking>(id, &ct);

    if (ct) {
        camera_id = ct->camera_id;
        min_hue = ct->min_hue;
        max_hue = ct->max_hue;
    }

    return result;
}

TFV_Result tfv::Api::colortracking_stop(TFV_Id id) {
    return component_stop<tinkervision::Colortracking>(id);
}

TFV_Result tfv::Api::colortracking_start(TFV_Id id) {
    return component_start<tinkervision::Colortracking>(id);
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
    auto result = TFV_CAMERA_ACQUISITION_FAILED;

    tinkervision::Component* component = nullptr;

    // Todo: add configuration first, see _start
    if (camera_control_.acquire(camera_id)) {

        result = TFV_FEATURE_CONFIGURATION_FAILED;

        auto it = components_.find(id);
        if (it != components_.end()) {  // reconfiguration

            auto cam_users = camera_control_.safe_release(camera_id);

            component = it->second;
            if (check_type<Comp>(component)) {
                if (component->camera_id != camera_id) {  // other cam
                    if (not cam_users) {  // camera no longer used. Todo:
                                          // Schedule
                                          // this
                        if (frames_.find(camera_id) != frames_.end()) {
                            std::lock_guard<std::mutex> lock(frame_lock_);
                            delete frames_[camera_id];
                            frames_.erase(camera_id);
                        }
                    }
                    int rows, columns, channels = 0;
                    camera_control_.get_properties(camera_id, rows, columns,
                                                   channels);
                    {
                        std::lock_guard<std::mutex> lock(frame_lock_);
                        frames_[camera_id] =
                            new Frame(camera_id, rows, columns, channels);
                    }
                }
                result = TFV_INVALID_CONFIGURATION;

                if (tinkervision::valid<Comp>(args...)) {
                    tinkervision::set<Comp>(static_cast<Comp*>(component),
                                            args...);

                    components_[id]->active = true;
                    result = TFV_OK;
                }
            }
        } else {  // new configuration
            result = TFV_INVALID_CONFIGURATION;
            {
                std::lock_guard<std::mutex> lock(components_lock_);
                components_[id] = new Comp(camera_id, args...);
            }
            if (tinkervision::valid<Comp>(args...)) {
                if (frames_.find(camera_id) == frames_.end()) {  // new cam
                    int rows, columns, channels = 0;
                    camera_control_.get_properties(camera_id, rows, columns,
                                                   channels);
                    {
                        std::lock_guard<std::mutex> lock(frame_lock_);
                        frames_[camera_id] =
                            new Frame(camera_id, rows, columns, channels);
                    }
                }
                components_[id]->active = true;
                result = TFV_OK;
            } else {
                {
                    std::lock_guard<std::mutex> lock(components_lock_);
                    delete components_[id];
                    components_.erase(id);  // Todo: only schedule deletion here
                    camera_control_.safe_release(camera_id);
                }
            }
        }
    }

    return result;
}

template <typename Component>
TFV_Result tfv::Api::component_get(TFV_Id id, Component** component) const {
    auto result = TFV_UNCONFIGURED_ID;
    auto it = components_.find(id);

    if (it != components_.end()) {
        result = TFV_INVALID_ID;

        if (check_type<Component>(it->second)) {
            result = TFV_OK;
            *component = static_cast<Component*>(it->second);
        }
    }

    return result;
}

template <typename Component>
TFV_Result tfv::Api::component_start(TFV_Id id) {
    auto result = TFV_UNCONFIGURED_ID;

    auto it = components_.find(id);
    if (it != components_.end()) {

        auto component = it->second;
        result = TFV_INVALID_ID;

        if (check_type<Component>(component)) {
            result = TFV_CAMERA_ACQUISITION_FAILED;

            if (camera_control_.acquire(component->camera_id)) {

                component->active = true;
                result = TFV_OK;

            } else {
                // Todo: if acquiring fails, there shouldn't be a need for
                // explicit release
                camera_control_.safe_release(component->camera_id);
            }
        }
    }
    return result;
}

template <typename Component>
TFV_Result tfv::Api::component_stop(TFV_Id id) {
    auto result = TFV_UNCONFIGURED_ID;

    auto it = components_.find(id);
    if (it != components_.end()) {

        auto const component = it->second;
        auto const camera_id = component->camera_id;
        component->active = false;
        auto users = camera_control_.safe_release(camera_id);
        if (not users) {
            if (frames_.find(camera_id) != frames_.end()) {
                std::lock_guard<std::mutex> lock(frame_lock_);
                delete frames_[camera_id];
                frames_.erase(camera_id);
            }
        }
        result = TFV_OK;
    }
    return result;
}
