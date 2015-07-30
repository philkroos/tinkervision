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
#include "window.hh"

tfv::Api::Api(void) { (void)start(); }

tfv::Api::~Api(void) { quit(); }

TFV_Result tfv::Api::start(void) {

    auto result = TFV_EXEC_THREAD_FAILURE;

    // Allow mainloop to run
    active_ = true;

    auto active_count = modules_.count(
        [](tfv::Module const& module) { return module.enabled(); });

    camera_control_.acquire(active_count);

    Log("API", "Restarting with ", active_count, " modules");

    // Start threaded execution of mainloop
    if (not executor_.joinable()) {
        executor_ = std::thread(&tfv::Api::execute, this);
    }

    if (executor_.joinable()) {
        result = TFV_OK;
    }

    return result;
}

TFV_Result tfv::Api::stop(void) {
    auto result = TFV_EXEC_THREAD_FAILURE;

    if (executor_.joinable()) {

        // Notify the threaded execution-context to stop and wait for it
        active_ = false;
        executor_.join();

        // Release all unused resources. Keep the modules' active state
        // unchanged, so that restarting the loop resumes known context.
        modules_.exec_all([this](
            TFV_Int id, tfv::Module& module) { camera_control_.release(); });
    }

    if (not executor_.joinable()) {
        result = TFV_OK;
    }

    // explicitly release the camera, in case sth. went wrong above
    camera_control_.release_all();

    return result;
}

TFV_Result tfv::Api::quit(void) {

    // stop all modules
    modules_.exec_all(
        [this](TFV_Int id, tfv::Module& module) { module.disable(); });

    // This included the dummy module
    idle_process_running_ = false;

    // stop execution of the main loop
    return stop();
}

void tfv::Api::execute(void) {

    // Execute active module. This is the ONLY place where modules are executed.
    auto module_exec = [&](TFV_Int id, tfv::Module& module) {

        // skip paused modules
        if (not module.enabled()) {
            return;
        }

        if (module.expected_format() != ColorSpace::NONE) {

            // retrieve the frame in the requested format and execute the module
            camera_control_.get_frame(image_, module.expected_format());
            module.exec(image_);
        }

        auto& tags = module.tags();
        if (tags & Module::Tag::Fx) {
            camera_control_.regenerate_formats_from(image_);
        }

        if (tags & Module::Tag::ExecAndRemove) {
            module.tag(Module::Tag::Removable);
            camera_control_.release();

        } else if (tags & Module::Tag::ExecAndDisable) {
            if (module.disable()) {
                camera_control_.release();
            }
        }
    };

    // mainloop
    unsigned const no_module_min_latency_ms = 200;
    unsigned const with_module_min_latency_ms = 50;
    auto latency_ms = no_module_min_latency_ms;
    while (active_) {

        // Activate new and remove freed resources
        modules_.update(nullptr);

        if (active_modules()) {  // This does not account for modules
            // being 'stopped', i.e. this is true even if all modules are in
            // paused state.  Then, camera_control_ will return the last
            // image retrieved from the camera (and it will be ignored by
            // update_module anyways)

            latency_ms =
                std::max(execution_latency_ms_, with_module_min_latency_ms);

            if (camera_control_.update_frame()) {

                if (not _scenes_active()) {
                    Log("API", "Executing all");
                    modules_.exec_all(module_exec);
                } else {
                    Log("API", "Executing tree");
                    scene_trees_.exec_all(); /*
                                     node_exec,
                                     camera_control_.latest_frame_timestamp());
                                 */
                }

            } else {
                // Log a warning
            }
        } else {
            latency_ms =
                std::max(execution_latency_ms_, no_module_min_latency_ms);
        }

        // Finally propagate deletion of modules marked for removal
        modules_.free_if([](tfv::Module const& module) {
            return module.tags() & Module::Tag::Removable;
        });

        // some buffertime for two reasons:
        // - the outer thread gets time to run
        // - the camera driver is probably not
        //   that fast and showed to fail if it is driven too fast.
        //
        // \todo The proper time to wait depends on the actual hardware and
        // should at least consider the actual time the modules
        // need to execute.

        std::this_thread::sleep_for(std::chrono::milliseconds(latency_ms));
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

__attribute__((constructor)) void startup(void) {
    // std::cout << "Constructing the Api on-demand" << std::endl;
    // nothing to do
    // api = new tfv::Api;
}

__attribute__((destructor)) void shutdown(void) {
    // std::cout << "De-Constructing the Api" << std::endl;
    if (api) {
        delete api;
    }
}
