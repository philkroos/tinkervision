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

#ifdef DEV
#include <opencv2/opencv.hpp>
#endif  // DEV

#include "api.hh"

tfv::Api::Api(void) : camera_control_{TFV_MAX_USERS_PER_CAM} { (void)start(); }

tfv::Api::~Api(void) {

    for (auto& frame : frames_) {
        if (frame.second) {
            delete frame.second;
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
        if (colortracker_.active()) {
            // refresh frames
            for (auto& frame : frames_) {
#ifdef DEV
                auto grabbed =
                    camera_control_.get_frame(frame.first, frame.second->data);
                if (grabbed and frame.first == 0) {  // show cam0
                    window.update(frame.second->data, frame.second->rows,
                                  frame.second->columns);
                }
#else
                (void)camera_control_.get_frame(frame.first,
                                                frame.second->data);
#endif
            }
            colortracker_.execute(frames_);
        } else {
            // no work currently, wait a sec
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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

    auto result = TFV_INVALID_CONFIGURATION;

    if (tfv::check_configuration_settings(min_hue, max_hue, callback)) {

        /*
          result = component_set(colortracker_, id,
          camera_id,
                                       min_hue, max_hue,
          callback, context);
        */
        result = TFV_CAMERA_ACQUISITION_FAILED;

        if (camera_control_.acquire(camera_id)) {

            result = TFV_FEATURE_CONFIGURATION_FAILED;

            if (colortracker_.add_configuration(id, camera_id, min_hue, max_hue,
                                                callback, context)) {

                colortracker_.activate_configuration(id);
                if (frames_.find(camera_id) == frames_.end()) {
                    int rows, columns, channels = 0;
                    camera_control_.get_properties(camera_id, rows, columns,
                                                   channels);
                    frames_[camera_id] =
                        new Frame(camera_id, rows, columns, channels);
                }
                result = TFV_OK;
            } else {
                camera_control_.safe_release(camera_id);
            }
        }
    }
    return result;
}

TFV_Result tfv::Api::colortracking_get(TFV_Id id, TFV_Id& camera_id,
                                       TFV_Byte& min_hue,
                                       TFV_Byte& max_hue) const {

    auto result = TFV_UNCONFIGURED_ID;

    auto configuration = colortracker_.get_configuration(id);
    if (configuration) {
        camera_id = configuration->camera_id;
        min_hue = configuration->min_hue;
        max_hue = configuration->max_hue;

        result = TFV_OK;
    }

    return result;
}

TFV_Result tfv::Api::colortracking_stop(TFV_Id id) {
    //    return component_stop(colortracker_, id);
    auto result = TFV_UNCONFIGURED_ID;

    auto configuration = colortracker_.get_configuration(id);
    if (configuration) {

        auto camera_id = configuration->camera_id;
        if (colortracker_.deactivate_configuration(id)) {

            auto users = camera_control_.safe_release(camera_id);
            if (not users) {
                if (frames_.find(camera_id) != frames_.end()) {
                    delete frames_[camera_id];
                    frames_.erase(camera_id);
                }
            }
            result = TFV_OK;
        }
    }
    return result;
}

TFV_Result tfv::Api::colortracking_start(TFV_Id id) {
    auto result = TFV_UNCONFIGURED_ID;

    auto configuration = colortracker_.get_configuration(id);
    if (configuration) {
        result = TFV_CAMERA_ACQUISITION_FAILED;

        if (camera_control_.acquire(configuration->camera_id)) {

            // already checked with get_configuration
            (void)colortracker_.activate_configuration(id);
            result = TFV_OK;
        } else {
            camera_control_.safe_release(configuration->camera_id);
        }
    }
    return result;
}

TFV_Result tfv::Api::is_camera_available(TFV_Id camera_id) {
    auto result = TFV_CAMERA_ACQUISITION_FAILED;
    if (camera_control_.is_available(camera_id)) {
        result = TFV_OK;
    }

    return result;
}

/*
TFV_Result tfv::Api::component_start (TVComponent& component, TFV_Id id) {
    auto result = TFV_UNCONFIGURED_ID;

    auto configuration = component.get_configuration(id);
    if (configuration) {
        result = TFV_CAMERA_ACQUISITION_FAILED;

        if (camera_control_.acquire(configuration->camera_id)) {

            // already checked with get_configuration
            (void)component.activate_configuration(id);
            result = TFV_OK;
        }
        else {
            camera_control_.safe_release(configuration->camera_id);
        }
    }
    return result;
}

template<typename... Args>
TFV_Result tfv::Api::component_set(TVComponent& component,
                                          TFV_Id id,
                                          TFV_Id camera_id,
                                          Args... args) {
    auto result = TFV_CAMERA_ACQUISITION_FAILED;

    if (camera_control_.acquire(camera_id)) {

        result = TFV_FEATURE_CONFIGURATION_FAILED;

        if (component.add_configuration(id, camera_id, args)) {

            component.activate_configuration(id);
            result = TFV_OK;
        }
        else {
            camera_control_.safe_release(camera_id);
        }
    }
    return result;
}

TFV_Result tfv::Api::component_stop (TVComponent& component, TFV_Id id) {
    auto result = TFV_UNCONFIGURED_ID;

    auto configuration = component.get_configuration(id);
    if (configuration) {

        auto camera_id = configuration->camera_id;
        if (component.deactivate_configuration(id)) {

            camera_control_.safe_release(camera_id);
            result = TFV_OK;
        }
    }
    return result;
}
*/
