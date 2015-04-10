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

tfv::Api::~Api(void) { quit(); }

TFV_Result tfv::Api::start(void) {

    auto result = TFV_EXEC_THREAD_FAILURE;

    // Allow mainloop to run
    active_ = true;

    auto active_count = modules_.count(
        [](tfv::Module const& module) { return module.enabled(); });

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
            TFV_Int id, tfv::Module& module) { camera_control_.release(); });
    }

    if (not executor_.joinable()) {
        result = TFV_OK;
    }

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

    // Execute active module
    auto update_module = [this](TFV_Int id, tfv::Module& module) {

        // skip paused modules
        if (not module.enabled()) {
            return;
        }

        auto tags = module.tags();
        if ((tags & Module::Tag::Executable) and
            (not chained_ or (tags & Module::Tag::Sequential))) {

            auto& executable = static_cast<Executable&>(module);
            // retrieve the frame in the requested format and execute the module
            camera_control_.get_frame(image_, executable.expected_format());
            executable.execute(image_);

            auto& tags = module.tags();
            if (tags & Module::Tag::ExecAndRemove) {
                module.tag(Module::Tag::Removable);
                camera_control_.release();

            } else if (tags & Module::Tag::ExecAndDisable) {
                if (module.disable()) {
                    camera_control_.release();
                }
            }
        }
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

        // Finally propagate deletion of modules marked for removal
        modules_.free_if([](tfv::Module const& module) {
            return module.tags() & Module::Tag::Removable;
        });

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

TFV_Result tfv::Api::chain(TFV_Id first, TFV_Id second) {

    auto result = TFV_INVALID_ID;

    if (first == second) {
        std::cout << "Won't chain identical" << std::endl;
        return result;
    }

    auto id_first = static_cast<TFV_Int>(first);
    auto id_second = static_cast<TFV_Int>(second);
    auto was_chained_ = chained_.fetch_or(1);
    std::cout << "Api::chain, chained: " << (bool)was_chained_ << std::endl;

    auto find_iter_before = [&](Modules::IdList const& list, TFV_Int element) {

        auto end = std::next(list.before_begin(),
                             std::distance(list.begin(), list.end()));

        for (auto it = list.before_begin(); it != end; std::advance(it, 1)) {

            if (*std::next(it) == element) {
                return it;
            }
        }

        return list.end();  // not found
    };

    // modules locked
    modules_.sort_manually([&](Modules::IdList& ids) {

        auto before_first = find_iter_before(ids, id_first);
        auto before_second = find_iter_before(ids, id_second);

        if (before_first == ids.end() or before_second == ids.end()) {
            return;
        }

        auto it_first = std::next(before_first);
        auto it_second = std::next(before_second);

        if (not was_chained_) {  // starting new chain

            std::cout << "New chain" << std::endl;
            ids.erase_after(before_first);
            ids.erase_after(before_second);
            ids.push_front(id_second);
            ids.push_front(id_first);
            modules_.access_unlocked(*it_first).tag(Module::Tag::Sequential);
            modules_.access_unlocked(*it_second).tag(Module::Tag::Sequential);

        } else if (modules_.access_unlocked(*it_second).tags() &
                   Module::Tag::Sequential) {  // second was already chained

            std::cout << "Second was chained" << std::endl;
            ids.erase_after(before_first);
            ids.insert_after(before_second, id_first);
            modules_.access_unlocked(*it_first).tag(Module::Tag::Sequential);

        } else if (modules_.access_unlocked(*it_first).tags() &
                   Module::Tag::Sequential) {  // first was already chained

            std::cout << "First was chained" << std::endl;
            ids.erase_after(before_second);
            ids.insert_after(it_first, id_second);
            modules_.access_unlocked(*it_second).tag(Module::Tag::Sequential);

        } else {  // none was chained. This is an error since a chain is
                  // active.
            std::cout << "Won't chain unchained" << std::endl;
            return;
        }

        if (modules_.access_unlocked(*it_first).enable()) {
            camera_control_.acquire();
        }
        if (modules_.access_unlocked(*it_second).enable()) {
            camera_control_.acquire();
        }

        auto unchained =
            std::find_if(ids.cbegin(), ids.cend(), [&](TFV_Int const& id) {
                return not(modules_.access_unlocked(id).tags() &
                           Module::Tag::Sequential);
            });

        for (; unchained != ids.end(); std::advance(unchained, 1)) {
            if (modules_.access_unlocked(*unchained).disable()) {
                camera_control_.release();
            }
        }

        result = TFV_OK;
    });

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
