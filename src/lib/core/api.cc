/// \file api.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Defines the internal interface of Tinkervision.
///
/// The interface is provided by the class Api.
///
/// This file is part of Tinkervision - Vision Library for Tinkerforge Redbrick
/// \sa https://github.com/Tinkerforge/red-brick
///
/// \copyright
///
/// This program is free software; you can redistribute it and/or
/// modify it under the terms of the GNU General Public License
/// as published by the Free Software Foundation; either version 2
/// of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
/// USA.

#include <sys/types.h>
#include <unistd.h>

#include "api.hh"
#include "module_wrapper.hh"

#ifndef USR_PREFIX
#error USR_PREFIX not defined
#endif

tv::Api::Api(void) noexcept(noexcept(CameraControl()) and
                            noexcept(FrameConversions()) and
                            noexcept(Strings()) and noexcept(SceneTrees())) {

    try {
        // dynamic construction because not noexcept on the RedBrick-platform
        environment_ = new Environment;

        if (not environment_->set_user_prefix(USR_PREFIX)) {
            throw ConstructionException("Environment", USR_PREFIX);
        }

        // dynamic construction because not noexcept
        modules_ = new Modules(&Api::module_exec, this);

        // dynamic construction because not noexcept
        module_loader_ = new ModuleLoader(*environment_);

        active_ = true;
        executor_ = std::thread(&Api::execute, this);

        if (not executor_.joinable()) {
            throw ConstructionException("Api", "Thread creation failed");
        }

        api_valid_ = true;

    } catch (ConstructionException const& e) {
        LogError("API", "Construction failed: ", e.what());
        active_ = false;
        if (executor_.joinable()) {
            executor_.join();
        }
    } catch (...) {
        LogError("API", "Construction failed!");
    }
}

tv::Api::~Api(void) {
    (void)quit();
    if (module_loader_) {
        delete module_loader_;
    }
    if (environment_) {
        delete environment_;
    }
    if (modules_) {
        delete modules_;
    }
}

bool tv::Api::valid(void) const { return api_valid_; }

int16_t tv::Api::start(void) {
    Log("API", "Restarting");

    if (not valid()) {
        return TV_INTERNAL_ERROR;
    }

    if (executor_.joinable()) {
        return TV_THREAD_RUNNING;
    }

    auto active_count = modules_->count(
        [](ModuleWrapper const& module) { return module.enabled(); });

    if (active_count == 0) {
        return TV_NO_ACTIVE_MODULES;
    }

    if (not camera_control_.acquire(active_count)) {
        return TV_CAMERA_NOT_AVAILABLE;
    }

    Log("API", "Restarting with ", active_count, " modules");

    // Start threaded execution of mainloop
    active_ = true;
    executor_ = std::thread(&Api::execute, this);

    if (not executor_.joinable()) {
        active_ = false;
        return TV_EXEC_THREAD_FAILURE;
    }

    return TV_OK;
}

int16_t tv::Api::stop(void) {

    if (not valid()) {
        return TV_OK;
    }

    auto active_count = modules_->count(
        [](ModuleWrapper const& module) { return module.enabled(); });

    Log("API", "Stopping with ", active_count, " modules");

    if (executor_.joinable()) {

        // Notify the threaded execution-context to stop and wait for it
        active_ = false;
        executor_.join();
    }

    Log("API::stop", "Execution thread stopped");

    camera_control_.release_all();

    Log("API::stop", "Camera released");

    return TV_OK;
}

int16_t tv::Api::quit(void) {
    Log("Api::quit");

    // ... release the camera and join the execution thread
    (void)stop();

    // ... remove all modules from the shared context ...
    if (valid()) {
        remove_all_modules();
    }

    // \todo assert that everything has been stopped.
    return TV_OK;
}

// Execute active module. This is the ONLY place where modules are executed.
void tv::Api::module_exec(int16_t id, ModuleWrapper& module) {
    // Log("API", "Executing module ", id);

    if (not module.enabled()) {  // skip paused modules
        return;
    }

    if (module.expected_format() !=
        ColorSpace::NONE) {  // retrieve the frame in the requested format
                             // and execute the module

        conversions_.get_frame(image_, module.expected_format());
        // ignoring result, doing callbacks (maybe, see default_callback_)
        try {
            module.execute(image_);
        } catch (...) {
            LogError("API", "Module ", module.name(), " (", id, ") crashed: ");
            module.tag(ModuleWrapper::Tag::Removable);
            return;
        }
    }

    auto& output = module.modified_image();
    if (output.header.format != ColorSpace::INVALID) {
        conversions_.set_frame(output);
    }

    auto& tags = module.tags();
    if (tags & ModuleWrapper::Tag::ExecAndRemove) {
        module.tag(ModuleWrapper::Tag::Removable);
        camera_control_.release();

    } else if (tags & ModuleWrapper::Tag::ExecAndDisable) {
        Log("API", "Disabling ExecAndDisable-tagged id ", module.id());
        module.disable();
        camera_control_.release();
    }
}

void tv::Api::execute(void) {
    Log("API", "Starting main loop");

    auto node_exec = [&](int16_t module_id) {
        (void)modules_->exec_one(module_id,
                                 [&module_id, this](ModuleWrapper& module) {
            module_exec(module_id, module);
            return TV_OK;
        });
    };

    // mainloop
    auto last_loop_time_point = Clock::now();
    auto loops = 0;
    auto loop_duration = Clock::duration(0);

    while (active_) {
        last_loop_time_point = Clock::now();
        // Log("API", "Execution at ", last_loop_time_point);

        Image frame;

        if (not paused_ and active_modules()) {  // active_modules() does not
                                                 // account for modules
            // being 'stopped', i.e. this is true even if all modules are in
            // paused state.  Then, camera_control_ will return the last
            // image retrieved from the camera (and it will be ignored by
            // update_module anyways)

            if (not camera_control_.update_frame(frame)) {
                LogWarning("API", "Could not retrieve the next frame");
            } else {
                conversions_.set_frame(frame);

                if (not _scenes_active()) {
                    modules_->exec_all();
                } else {
                    scene_trees_.exec_all(
                        node_exec, camera_control_.latest_frame_timestamp());
                }
            }

            // Propagate deletion of modules marked for removal
            modules_->remove_if([](ModuleWrapper const& module) {
                return module.tags() & ModuleWrapper::Tag::Removable;
            });

            loops++;
            loop_duration += (Clock::now() - last_loop_time_point);
            if (loops == 10) {
                effective_frameperiod_ =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        loop_duration).count() /
                    10.0;

                loops = 0;
                loop_duration = Clock::duration(0);
            }
            // sleep for the duration requested, if it's not in the past
            std::this_thread::sleep_until(
                last_loop_time_point +
                std::chrono::milliseconds(frameperiod_ms_));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    Log("API", "Mainloop stopped");
}

int16_t tv::Api::module_run_now(int8_t id) {
    return modules_->exec_one_now(id, [this, id](ModuleWrapper& module) {
        assert(module.enable_at_least_once());
        module_exec(id, module);
        return TV_OK;
    });
}

int16_t tv::Api::module_run_now_new_frame(int8_t id) {
    return modules_->exec_one_now_restarting(id,
                                             [this, id](ModuleWrapper& module) {
        Image frame;
        assert(module.enable_at_least_once());
        if (not camera_control_.update_frame(frame)) {
            LogWarning("API", "Could not retrieve the next frame");
            return TV_CAMERA_NOT_AVAILABLE;
        }

        conversions_.set_frame(frame);
        module_exec(id, module);
        return TV_OK;
    });
}

int16_t tv::Api::set_framesize(uint16_t width, uint16_t height) {

    auto result = TV_CAMERA_SETTINGS_FAILED;

    /// \todo Count the active modules continuously and remove code dup.
    auto active_count = modules_->count(
        [](tv::ModuleWrapper const& module) { return module.enabled(); });

    if (active_count) {
        uint16_t w, h;
        camera_control_.get_resolution(w, h);

        if (w != width or h != height) {  // different settings?
            auto code = stop();
            if (code != TV_OK) {
                LogError("API", "SetFramesize ", "Stop returned ", code);
            }

            /// If the settings can't be applied, any previous ones will
            /// be restored.
            if (camera_control_.preselect_framesize(width, height)) {
                result = TV_OK;
            }

            code = start();
            if (code != TV_OK) {
                LogError("API", "SetFramesize ", "Start returned ", code);
            }
        } else {
            result = TV_OK;
        }
    } else {  // camera not running
        if (camera_control_.preselect_framesize(width, height)) {
            result = TV_OK;
        }
    }

    return result;
}

int16_t tv::Api::start_idle(void) {
    auto result = TV_OK;  // optimistic because startable only once

    if (not idle_process_running_) {
        result = _module_load("dummy", _next_internal_id());
    }
    idle_process_running_ = (result == TV_OK);
    return result;
}

int16_t tv::Api::module_load(std::string const& name, int8_t& id) {
    auto module_id = _next_public_id();

    assert(module_id < std::numeric_limits<int8_t>::max() and module_id > 0);

    auto result = _module_load(name, static_cast<int16_t>(module_id));

    if (TV_INVALID_ID == result) {
        // this is an unhandled id clash, see _next_public_id
        result = TV_INTERNAL_ERROR;
    }

    if (TV_OK == result) {
        id = static_cast<int8_t>(module_id);
    }
    return result;
}

int16_t tv::Api::module_destroy(int8_t id) {
    Log("API", "Destroying module ", id);

    /// - no scenes are active (currently). Then,
    if (_scenes_active()) {
        return TV_NOT_IMPLEMENTED;
    }

    /// the module will be disabled and registered for removal which
    /// will
    /// happen in the main execution loop.
    /// \todo Is a two-stage-removal process still necessary now that
    /// the allocation stage was removed from SharedResource? Not sure,
    /// probably not.
    return modules_->exec_one_now(id, [this](tv::ModuleWrapper& module) {
        module.disable();
        module.tag(ModuleWrapper::Tag::Removable);
        camera_control_.release();
        return TV_OK;
    });
}

int16_t tv::Api::module_start(int8_t module_id) {
    auto id = static_cast<int16_t>(module_id);

    if (not modules_->managed(id)) {
        return TV_INVALID_ID;
    }

    return _enable_module(module_id);
}

int16_t tv::Api::module_stop(int8_t module_id) {
    Log("API", "Stopping module ", module_id);

    auto id = static_cast<int16_t>(module_id);

    if (not modules_->managed(id)) {
        return TV_INVALID_ID;
    }

    return _disable_module(module_id);
}

char const* tv::Api::result_string(int16_t code) const {
    return result_string_map_[code];
}

bool tv::Api::is_camera_available(void) {
    return camera_control_.is_available();
}

bool tv::Api::is_camera_available(uint8_t id) {
    return camera_control_.is_available(id);
}

bool tv::Api::prefer_camera_with_id(uint8_t id) {
    return camera_control_.switch_to_preferred(id);
}

int16_t tv::Api::resolution(uint16_t& width, uint16_t& height) {
    return camera_control_.get_resolution(width, height)
               ? TV_OK
               : TV_CAMERA_NOT_AVAILABLE;
}

int16_t tv::Api::request_frameperiod(uint32_t ms) {
    frameperiod_ms_ = ms;
    return TV_OK;
}

int16_t tv::Api::module_get_name(int8_t module_id, std::string& name) const {
    if (not modules_->managed(module_id)) {
        return TV_INVALID_ID;
    }

    name = (*modules_)[module_id]->name();
    return TV_OK;
}

int16_t tv::Api::module_is_active(int8_t module_id, bool& active) const {
    if (not modules_->managed(module_id)) {
        return TV_INVALID_ID;
    }

    active = (*modules_)[module_id]->enabled();
    return TV_OK;
}

size_t tv::Api::loaded_libraries_count(void) const { return modules_->size(); }

int16_t tv::Api::module_id(size_t library, int8_t& id) const {
    id = static_cast<int8_t>(modules_->managed_id(library));
    return (id != TV_INVALID_ID ? TV_OK : TV_INVALID_ARGUMENT);
}

int16_t tv::Api::library_get_parameter_count(std::string const& libname,
                                             uint16_t& count) const {
    if (module_loader_->library_parameter_count(libname, count)) {
        return TV_OK;
    }
    return TV_INVALID_ARGUMENT;
}

int16_t tv::Api::library_describe_parameter(std::string const& libname,
                                            size_t parameter, std::string& name,
                                            uint8_t& type, int32_t& min,
                                            int32_t& max, int32_t& def) {

    Parameter const* p;
    if (not module_loader_->library_get_parameter(libname, parameter, &p)) {
        return TV_INVALID_ARGUMENT;
    }

    name = p->name();
    type = p->type() == Parameter::Type::String ? 1 : 0;

    if (0 == type) {  // numerical parameter
        if (not(p->get(def))) {
            return TV_INTERNAL_ERROR;
        }
        min = p->min();
        max = p->max();
    }

    return TV_OK;
}

int16_t tv::Api::module_enumerate_parameters(int8_t module_id,
                                             TV_StringCallback callback,
                                             void* context) const {
    if (not modules_->managed(module_id)) {
        return TV_INVALID_ID;
    }

    std::vector<Parameter const*> parameters;
    (*modules_)[module_id]->get_parameters_list(parameters);

    if (parameters.size()) {
        std::thread([module_id, parameters, callback, context](void) {
                        for (auto const& par : parameters) {
                            callback(module_id, par->name().c_str(), context);
                        }
                        callback(0, "", context);  // done
                    }).detach();
    }

    return TV_OK;
}

int16_t tv::Api::libraries_changed_callback(TV_LibrariesCallback callback,
                                            void* context) {

    module_loader_->update_on_changes([callback, context](
        std::string const& dir, std::string const& file,
        Dirwatch::Event event) {
        auto const status = (event == Dirwatch::Event::FILE_CREATED ? 1 : -1);
        callback(file.c_str(), dir.c_str(), status, context);
    });
    return TV_OK;
}

int16_t tv::Api::set_user_paths_prefix(std::string const& path) {
    auto old_modules_path = environment_->user_modules_path();
    auto result = environment_->set_user_prefix(path);

    // notify the environment to update the list of available modules.
    (void)module_loader_->switch_user_load_path(
        old_modules_path, environment_->user_modules_path());

    return result ? TV_OK : TV_INVALID_ARGUMENT;
}

int16_t tv::Api::callback_set(int8_t module_id, TV_Callback callback) {
    if (default_callback_ != nullptr) {
        return TV_GLOBAL_CALLBACK_ACTIVE;
    }

    if (not(*modules_)[module_id]) {
        return TV_INVALID_ID;
    }

    auto& module = *(*modules_)[module_id];

    if (not module.register_callback(callback)) {
        LogError("API", "Could not set callback for module ", module.name());
        return TV_INTERNAL_ERROR;
    }

    return TV_OK;
}

int16_t tv::Api::callback_default(TV_Callback callback) {
    default_callback_ = callback;

    modules_->exec_all([&callback](int16_t id, ModuleWrapper& module) {
        (void)module.register_callback(callback);
    });

    return TV_OK;
}

int16_t tv::Api::get_result(int8_t module_id, TV_ModuleResult& result) {

    return modules_->exec_one_now(module_id, [&](ModuleWrapper& module) {
        auto res = module.result();
        if (not res) {
            return TV_RESULT_NOT_AVAILABLE;
        }

        result.x = res.x;
        result.y = res.y;
        result.width = res.width;
        result.height = res.height;
        std::strncpy(result.string, res.result.c_str(), TV_STRING_SIZE - 1);
        result.string[TV_STRING_SIZE - 1] = '\0';
        return TV_OK;
    });
}

uint32_t tv::Api::effective_frameperiod(void) const {
    return effective_frameperiod_;
}

std::string const& tv::Api::user_paths_prefix(void) const {
    return environment_->user_prefix();
}

std::string const& tv::Api::system_module_path(void) const {
    return environment_->system_modules_path();
}

/// Disable and remove all modules.
void tv::Api::remove_all_modules(void) {
    _disable_all_modules();

    modules_->remove_all();
    idle_process_running_ = false;
    Log("Api", "All modules released");
}

void tv::Api::get_libraries_count(uint16_t& count) const {
    count = module_loader_->libraries_count();
}

bool tv::Api::library_get_name_and_path(uint16_t count, std::string& name,
                                        std::string& path) const {
    return module_loader_->library_name_and_path(count, name, path);
}

/*
 * Private methods
 */

int16_t tv::Api::_module_load(std::string const& name, int16_t id) {
    Log("API", "ModuleLoad ", name, " ", id);
#ifndef DEFAULT_CALL
    uint16_t ms = DELAY_GRAIN * (GRAINS - 2);
#else
    uint16_t ms = 1000;
#endif
    auto start = std::chrono::steady_clock::now();
    auto maxend = start + std::chrono::milliseconds(ms);

    if ((*modules_)[id]) {
        return TV_INVALID_ID;
    }

    auto module = (ModuleWrapper*)(nullptr);
    if (not module_loader_->load_module_from_library(&module, name, id)) {
        Log("API", "Loading library ", name, " failed");
        return module_loader_->last_error();
    }

    if (not module->initialize()) {
        Log("API", "Initializing library ", name, " failed");
        module_loader_->destroy_module(module);
        return TV_MODULE_INITIALIZATION_FAILED;
    }

    if (not camera_control_.acquire()) {
        return TV_CAMERA_NOT_AVAILABLE;
    }

    // Add modules to managed objects and register destruction handler
    std::thread(
        [this, module, id](void) {
            if (not modules_->insert(id, module, [this](ModuleWrapper& module) {
                    module_loader_->destroy_module(&module);
                })) {

                camera_control_.release();
                LogError("API", "Inserting a module failed");
                return;
                // return TV_MODULE_INITIALIZATION_FAILED;
            }
            done_.test_and_set();
        }).detach();

    std::this_thread::sleep_until(maxend);
    if (done_.test_and_set()) {  // module loaded
        std::thread([this, module](void) {
                        /// Add the default callback to each new module
                        if (default_callback_) {
                            module->register_callback(default_callback_);
                        }

                        /// \todo Catch the cases in which this fails and remove
                        /// the
                        /// module.
                        (void)module->enable();
                    }).detach();
        return TV_OK;
    } else {
        std::thread([this, id](void) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(
                            2000));  // should be loaded now
                        module_destroy(id);
                    }).detach();

        return TV_BUSY;
    }
}

void tv::Api::_disable_all_modules(void) {

    paused_ = true;
    modules_->interrupt();
    modules_->exec_all([this](int16_t id, tv::ModuleWrapper& module) {
        module.disable();
        camera_control_.release();
    });
    paused_ = false;
}

void tv::Api::_disable_module_if(
    std::function<bool(tv::ModuleWrapper const& module)> predicate) {

    paused_ = true;
    modules_->interrupt();
    modules_->exec_all(
        [this, &predicate](int16_t id, tv::ModuleWrapper& module) {
            if (predicate(module)) {
                module.disable();
                camera_control_.release();
            }
        });
    paused_ = false;
}

void tv::Api::_enable_all_modules(void) {
    paused_ = true;
    modules_->interrupt();
    modules_->exec_all([this](int16_t id, tv::ModuleWrapper& module) {
        if (not module.enabled()) {
            if (camera_control_.acquire()) {
                module.enable();
            }
        }
    });
    paused_ = false;
}

int16_t tv::Api::_enable_module(int16_t id) {
    return modules_->exec_one_now(id, [this](tv::ModuleWrapper& module) {
        if (module.enabled() or camera_control_.acquire()) {
            module.enable();  // possibly redundant
            return TV_OK;
        } else {
            return TV_CAMERA_NOT_AVAILABLE;
        }
    });
}

int16_t tv::Api::_disable_module(int16_t id) {
    return modules_->exec_one_now(id, [this](tv::ModuleWrapper& module) {
        module.disable();
        camera_control_.release();
        return TV_OK;
    });
}

int16_t tv::Api::_next_public_id(void) const {
    static int8_t public_id{0};
    if (++public_id == 0) {
        public_id = 1;
        LogWarning("API", "Overflow of public ids");
    }
    return static_cast<int16_t>(public_id);
}

int16_t tv::Api::_next_internal_id(void) const {
    static int16_t internal_id{std::numeric_limits<int8_t>::max() + 1};
    return internal_id++;
}

/*
 * Lifetime management
 */

static tv::Api* api = nullptr;

tv::Api& tv::get_api(void) {
    static_assert(noexcept(Api()), "Api is not noexcept");

    if (not api) {
        api = new Api;
    }

    return *api;
}

__attribute__((constructor)) void startup(void) { tv::Log("API", "Create"); }

__attribute__((destructor)) void shutdown(void) {
    tv::Log("API", "Shutdown");
    if (api) {
        delete api;
    }
}
