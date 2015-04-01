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

#include "api.hh"
#include "module.hh"

tfv::Api::Api(void) { (void)start(); }

tfv::Api::~Api(void) { quit(); }

TFV_Result tfv::Api::start(void) {

    auto result = TFV_EXEC_THREAD_FAILURE;

    // Allow mainloop to run
    active_ = true;

    auto active_count =
        modules_.count([](tfv::Module const& module) { return module.active; });

    camera_control_.acquire(active_count);

    std::cout << "Restarting with " << active_count << " modules" << std::endl;

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
            TFV_Id id, tfv::Module& module) { camera_control_.release(); });
    }

    if (not executor_.joinable()) {
        result = TFV_OK;
    }

    return result;
}

TFV_Result tfv::Api::quit(void) {

    // stop all modules
    modules_.exec_all(
        [this](TFV_Id id, tfv::Module& module) { module.active = false; });

    // stop execution of the main loop
    return stop();
}

void tfv::Api::execute(void) {

    // Execute active module
    auto update_module = [this](TFV_Id id, tfv::Module& module) {

        // skip paused modules
        if (not module.active) {
            return;
        }

        // retrieve the frame in the requested format and execute the module
        camera_control_.get_frame(image_, module.expected_format());
        module.execute(image_);
    };

    // mainloop
    unsigned const no_module_min_latency_ms = 200;
    unsigned const with_module_min_latency_ms = 50;
    auto latency_ms = no_module_min_latency_ms;
    while (active_) {

        // Activate new and remove freed resources
        modules_.update();

        if (active_modules()) {  // This does not account for modules
            // being 'stopped', i.e. this is true even if all modules are in
            // paused state.  Then, camera_control_ will return the last image
            // retrieved from the camera (and it will be ignored by
            // update_module anyways)

            latency_ms =
                std::max(execution_latency_ms_, with_module_min_latency_ms);

            if (camera_control_.update_frame()) {

                modules_.exec_all(update_module);
            } else {
                // Log a warning
            }

        } else {
            latency_ms =
                std::max(execution_latency_ms_, no_module_min_latency_ms);
        }

        // some buffertime for two reasons: first, the outer thread
        // gets time to run second, the camera driver is probably not
        // that fast and will fail if it is driven too fast.
        //
        // \todo The proper time to wait depends on the actual hardware and
        // should at least consider the actual time the modules
        // need to execute.

        std::this_thread::sleep_for(std::chrono::milliseconds(latency_ms));
    }
}

TFV_Result tfv::Api::is_camera_available(void) {
    auto result = TFV_CAMERA_ACQUISITION_FAILED;
    if (camera_control_.is_available()) {
        result = TFV_OK;
    }

    return result;
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
    std::cout << "Constructing the Api on-demand" << std::endl;
    // nothing to do
    // api = new tfv::Api;
}

__attribute__((destructor)) void shutdown(void) {
    std::cout << "De-Constructing the Api" << std::endl;
    if (api) {
        delete api;
    }
}
