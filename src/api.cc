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
    auto update_frame = [this](TFV_Id id, tfv::FrameWithUserCounter& frame) {
        auto grabbed = camera_control_.get_frame(id, frame.data());
        if (grabbed) {
            window.update(id, frame.data(), frame.rows(), frame.columns());
        } else {
            std::cout << "Could not grab frame" << std::endl;
        }
    };

#else
    // Refresh camera
    auto update_frame = [this](TFV_Id id, tfv::FrameWithUserCounter& frame) {
        camera_control_.get_frame(id, frame.data());
    };
#endif

    // Execute active component
    auto const& frames = const_cast<Frames const&>(frames_);
    auto update_component = [this, &frames](TFV_Id id,
                                            tfv::Component& component) {
        if (component.active) {
            auto const& cam = component.camera_id;
            if (frames.managed(cam)) {

                auto const& frame = frames[cam];
                component.execute(frame.data(), frame.rows(), frame.columns());
            }
        }
    };

    // Stop component
    auto stop_component = [this](TFV_Id id, tfv::Component& component) {
        component.active = false;
        release_frame(component.camera_id);
    };

    // mainloop
    while (active_) {
        frames_.exec_all(update_frame);
        components_.exec_all(update_component);
        // Activate new and remove freed resources
        frames_.update();
        components_.update();
    }

    components_.exec_all(stop_component);  // this will also free the cameras
}

tfv::Api& tfv::get_api(void) {
    static Api* api = nullptr;

    // It would be easier to just return a static
    // Api-instance from get_api.
    // However, since the library is supposed to be run in
    // the context of a daemon, the calling context potentially
    // does not finish for a long time.
    // Therefore, watching the Api-usage with the following
    // static inner context makes it possible to release the
    // Api-ressource after some 'timeout'.
    // For the user, this is transparent since the Api will
    // only be released if it is inactive, in which case
    // reinstantiating the Api will have the same result as
    // would have accessing a 'sleeping' one.
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

TFV_Result tfv::Api::is_camera_available(TFV_Id camera_id) {
    auto result = TFV_CAMERA_ACQUISITION_FAILED;
    if (camera_control_.is_available(camera_id)) {
        result = TFV_OK;
    }

    return result;
}

void tfv::Api::allocate_frame(TFV_Id camera_id) {
    auto frame = frames_[camera_id];

    if (frame) {
        frame->user++;
    } else {
        int rows, columns, channels = 0;
        camera_control_.get_properties(camera_id, rows, columns, channels);
        auto const user = 1;
        frames_.allocate(camera_id, camera_id, rows, columns, channels, user);
    }
}

void tfv::Api::release_frame(TFV_Id camera_id) {
    auto frame =
        frames_[camera_id];  // takes too long sometimes, see shared_resource
    std::cout << "Release frame" << std::endl;
    if (frame) {
        frame->user--;
        if (not frame->user) {
            std::cout << "Release-free" << std::endl;
            frames_.free(camera_id);
            camera_control_.release(camera_id);
        }
    }
}
