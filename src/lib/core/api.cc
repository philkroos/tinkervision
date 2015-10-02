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

#include <sys/types.h>
#include <unistd.h>
#include <iterator>

#include "api.hh"
#include "module.hh"

tfv::Api::Api(void) { (void)start(); }

tfv::Api::~Api(void) { (void)quit(); }

TFV_Result tfv::Api::start(void) {
    Log("API", "Restarting");

    if (executor_.joinable()) {
        return TFV_THREAD_RUNNING;
    }

    auto active_count = modules_.count(
        [](tfv::Module const& module) { return module.enabled(); });

    if (active_count == 0) {
        return TFV_NO_ACTIVE_MODULES;
    }

    if (not camera_control_.acquire(active_count)) {
        return TFV_CAMERA_ACQUISITION_FAILED;
    }

    Log("API", "Restarting with ", active_count, " modules");

    // Start threaded execution of mainloop
    active_ = true;
    executor_ = std::thread(&tfv::Api::execute, this);

    if (not executor_.joinable()) {
        active_ = false;
        return TFV_EXEC_THREAD_FAILURE;
    }

    return TFV_OK;
}

TFV_Result tfv::Api::stop(void) {

    if (executor_.joinable()) {

        // Notify the threaded execution-context to stop and wait for it
        active_ = false;
        executor_.join();
    }

    Log("API::stop", "Execution thread stopped");

    camera_control_.release_all();

    Log("API::stop", "Camera released");

    return TFV_OK;
}

TFV_Result tfv::Api::quit(void) {
    Log("Api::quit");

    // disable all ...
    modules_.exec_all(
        [this](TFV_Int id, tfv::Module& module) { module.disable(); });

    // ... remove all modules from the shared context ...
    modules_.free_all();

    Log("Api", "All modules released");

    // (This included the dummy module)
    idle_process_running_ = false;

    // ... free all loaded libraries ...
    module_loader_.destroy_all();

    // ... release the camera and join the execution thread
    (void)stop();

    // \todo assert that everything has been stopped.
    return TFV_OK;
}

void tfv::Api::execute(void) {

    auto image = Image();

    // Execute active module. This is the ONLY place where modules are executed.
    auto module_exec = [&](TFV_Int id, tfv::Module& module) {

        if (not module.enabled() or
            not module.running()) {  // skip paused modules and those that don't
                                     // want to execute
            return;
        }

        if (module.expected_format() !=
            ColorSpace::NONE) {  // retrieve the frame in the requested format
                                 // and execute the module

            conversions_.get_frame(image, module.expected_format());

            module.exec(image);
        }

        if (module.type() == ModuleType::Modifier) {
            auto modifier = static_cast<Modifier*>(module.executable());
            conversions_.set_frame(modifier->modified_image());
        }

        auto& tags = module.tags();
        if (tags & Module::Tag::ExecAndRemove) {
            module.tag(Module::Tag::Removable);
            camera_control_.release();

        } else if (tags & Module::Tag::ExecAndDisable) {
            Log("API", "Disabling ExecAndDisable-tagged id ", module.id());
            if (module.disable()) {
                camera_control_.release();
            }
        }
    };

    auto node_exec = [&](TFV_Int module_id) {
        (void)modules_.exec_one(module_id,
                                [&module_id, &module_exec](Module& module) {
            module_exec(module_id, module);
            return TFV_OK;
        });
    };

    // mainloop
    auto inv_framerate = std::chrono::milliseconds(execution_latency_ms_);
    auto last_loop_time_point = Clock::now();
    auto duration = Clock::duration(0);
    auto loops = 0;
    while (active_) {
        last_loop_time_point = Clock::now();
        // Log("API", "Execution at ", last_loop_time_point);

        Image frame;
        if (active_modules()) {  // This does not account for modules
            // being 'stopped', i.e. this is true even if all modules are in
            // paused state.  Then, camera_control_ will return the last
            // image retrieved from the camera (and it will be ignored by
            // update_module anyways)

            if (camera_control_.update_frame(frame)) {
                conversions_.set_frame(frame);

                if (not _scenes_active()) {
                    modules_.exec_all(module_exec);
                } else {
                    scene_trees_.exec_all(
                        node_exec, camera_control_.latest_frame_timestamp());
                }

            } else {
                LogWarning("API", "Could not retrieve the next frame");
            }
        }

        // Propagate deletion of modules marked for removal
        modules_.free_if([](tfv::Module const& module) {
            return module.tags() & Module::Tag::Removable;
        });

        loops++;
        duration += (Clock::now() - last_loop_time_point);
        if (loops == 10) {
            Log("API", "Avg. real vs. fixed framerate: ",
                std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                        .count() /
                    10.0,
                "/", inv_framerate.count());
            loops = 0;
            duration = Clock::duration(0);
        }
        std::this_thread::sleep_until(last_loop_time_point + inv_framerate);
    }
}

/*
 * Lifetime management
 */

static tfv::Api* api = nullptr;

tfv::Api& tfv::get_api(void) {
    if (not api) {
        api = new Api;
    }

    return *api;
}

__attribute__((constructor)) void startup(void) { tfv::Log("API", "Create"); }

__attribute__((destructor)) void shutdown(void) {
    tfv::Log("API", "Shutdown");
    if (api) {
        delete api;
    }
}
