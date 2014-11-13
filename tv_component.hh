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

#ifndef TV_COMPONENT_H
#define TV_COMPONENT_H

#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <algorithm>

#include "frame.hh"

namespace tfv {

template <typename Configuration>
class TVComponent {
private:
    using ConfigurationId = TFV_Id;
    using CameraId = TFV_Id;
    //        using Configurations = std::unordered_map<ConfigurationId,
    // Configuration*>;
    //        using CameraMapping = std::unordered_map<ConfigurationId,
    // CameraId>;

    using Mapping = struct Mapping_ {
        //            Configuration* configuration;
        ConfigurationId configuration_id;
        CameraId camera_id;
        bool active;
        Mapping_(ConfigurationId configuration_id, CameraId camera_id)
            : configuration_id(configuration_id),
              camera_id(camera_id),
              active(false) {}
    };

    using Configurations = std::unordered_map<Mapping*, Configuration*>;

protected:
    bool active_ = false;

    virtual void on_execute(tfv::Frame const& frame,
                            Configuration const& configuration) = 0;
    virtual void after_configuration_added(
        Configuration const& configuration) = 0;
    virtual void after_configuration_removed(
        Configuration const& configuration) = 0;

public:
    TVComponent(void) = default;
    virtual ~TVComponent(void) {

        for (auto& configuration : configurations_) {
            // delete configuration.second;
            if (configuration.second) {
                delete configuration.second;
            }
            if (configuration.first) {
                delete configuration.first;
            }
        }
    }

    template <typename... Args>
    void get_config(Args&... args) {
        get_values(args...);
    }

    void get_values(int& a, int& b) {
        a = 4;
        b = 6;
    }

    void execute(tfv::Frames const& frames) {
        if (active_) {
            for (auto const& configuration : configurations_) {
                //                    if (configuration.second) {
                //                        on_execute (frames,
                // *(configuration.second));
                if (configuration.first->active) {
                    auto it = frames.find(configuration.first->camera_id);
                    if (it != frames.end()) {
                        on_execute(*(it->second), *(configuration.second));
                    }
                }
            }
        }
    }
    /*
                for (auto it = active_configurations_.begin();
                     it != active_configurations_.end();
                     ++it) {

                    if (not it->second) {
                        auto configuration = configurations_[it->first];
                        configurations_.erase(it->first);
                        after_configuration_removed (*configuration);
                        delete configuration;
                        active_configurations_.erase(it);
                    }
                }
            }
    */
    /*
            template<typename... Args>
            bool add_configuration (TFV_Id configuration_id, TFV_Id camera_id,
       Args... args)  {

                auto result = configurations_.find(configuration_id) !=
       configurations_.end();

                if (not result) {  // add new config
                    configurations_[configuration_id] = new
       Configuration(camera_id, args...);
                    active_configurations_[configuration_id] = INACTIVE;
                    after_configuration_added
       (*(configurations_[configuration_id]));
                }

                return not result;
            }
    */
    bool configuration_exists(ConfigurationId id) const {
        return configurations_.end() !=
               find_if(configurations_.begin(), configurations_.end(),
                       [id](std::pair<Mapping*, Configuration*> const&
                                configuration) {
                   return configuration.first->configuration_id == id;
               });
    }

    Mapping* find_configuration(ConfigurationId id) const {
        Mapping* result = nullptr;
        auto it = find_if(
            configurations_.begin(), configurations_.end(),
            [id](std::pair<Mapping*, Configuration*> const& configuration) {
                return configuration.first->configuration_id == id;
            });
        if (it != configurations_.end()) {
            result = it->first;
        }
        return result;
    }

    template <typename... Args>
    bool add_configuration(TFV_Id configuration_id, TFV_Id camera_id,
                           Args... args) {

        auto result = configuration_exists(configuration_id);

        if (not result) {  // add new config
            // configurations_[configuration_id] = new Configuration(camera_id,
            // args...);
            // active_configurations_[configuration_id] = INACTIVE;
            // auto config = new Configuration(camera_id, args...);
            auto mapping = new Mapping(configuration_id, camera_id);
            configurations_[mapping] = new Configuration(camera_id, args...);
            //  insert(std::pair<Mapping*, Configuration*> (config, mapping));
            // after_configuration_added (*(configurations_[configuration_id]));
        }

        return not result;
    }

    bool remove_configuration(TFV_Id id) {
        // auto result = configurations_.find(id) != configurations_.end();
        auto result = find_configuration(id);

        if (result) {
            // schedule removal
            result->active = false;
            // active_configurations_[id] = INACTIVE;
        }

        // return result;
        return result != nullptr;
    }

    Configuration* get_configuration(TFV_Id id) const {
        auto result = find_configuration(id);
        if (result) {
            return configurations_.find(result)->second;
        } else {
            return nullptr;
        }
    }

    /*
            bool get_configuration (TFV_Id id, Configuration& configuration)
       const {
                auto result = false;
                auto config = find_configuration(id);
                if (config) {
                    configuration = *(configurations_.find(config)->second);
                    result = true;
                }
                return result;
            }

            bool activate_configuration (TFV_Id id) {
                auto result = false;
                if (active_configurations_.find(id) !=
       active_configurations_.end()) {
                    active_configurations_[id] = true;
                    result = true;
                }

                return result;
            }
    */
    bool activate_configuration(TFV_Id id) {
        auto result = false;
        auto configuration = find_configuration(id);
        if (configuration) {
            configuration->active = true;
            result = true;
        }

        return result;
    }

    /*
            bool deactivate_configuration (TFV_Id id) {
                auto result = false;
                if (active_configurations_.find(id) !=
       active_configurations_.end()) {
                    active_configurations_[id] = true;
                    result = true;
                }

                return result;
            }
    */
    bool deactivate_configuration(TFV_Id id) {
        auto result = false;
        auto configuration = find_configuration(id);
        if (configuration) {
            configuration->active = false;
            result = true;
        }

        return result;
    }

    /*
            bool active (void) {
                return std::any_of (active_configurations_.begin(),
                                    active_configurations_.end(),
                                    [] (ActiveConfigurations::const_iterator
       const& it) {
                                        return it->second;
                                    });
            }
    */
    bool active(void) {
        return std::any_of(
            configurations_.begin(), configurations_.end(),
            [](std::pair<Mapping*, Configuration*> const& configuration) {
                return configuration.first->active;
            });
    }

private:
    std::mutex mutex_;
    Configurations configurations_;
    //        ActiveConfigurations active_configurations_;
};
};

#endif
