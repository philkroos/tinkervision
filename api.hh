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

#include <map>
#include <thread>
#include <mutex>

#include "strings.hh"
#include "tinkervision_defines.h"
#include "feature.hh"
#include "tv_component.hh"
#include "cameracontrol.hh"
#include "colortracker.hh"

#ifdef DEV
#include "window.hh"
#endif  // DEV

namespace tfv {

class Api {
private:
    Api(void);
    friend tfv::Api& get_api(void);
    bool active_components(void) { return active_ and colortracker_.active(); }

public:
    Api(Api const&) = delete;
    Api& operator=(Api const&) = delete;

    ~Api(void);

    bool start(void);
    bool stop(void);

    TFV_Result colortracking_set(TFV_Id id, TFV_Id cam_id, TFV_Byte min_hue,
                                 TFV_Byte max_hue, TFV_Callback callback,
                                 TFV_Context context);

    TFV_Result colortracking_get(TFV_Id id, TFV_Id& cam_id, TFV_Byte& min_hue,
                                 TFV_Byte& max_hue) const;

    TFV_Result colortracking_stop(TFV_Id id);
    TFV_Result colortracking_start(TFV_Id id);

    TFV_String result_string(TFV_Id code) const {
        return result_string_map_[code];
    }

    TFV_Bool is_camera_available(TFV_Id camera_id);

private:
    CameraControl camera_control_;
    TFVStringMap result_string_map_;

    Frames frames_;

    Colortracker colortracker_;

    std::thread executor_;
    void execute(void);
    bool active_ = true;

#ifdef DEV

    Window window{"API"};

#endif  // DEV
    /*
    TFV_Result component_stop (TVComponent& component, TFV_Id id);

    template<typename... Args>
    TFV_Result component_set(TVComponent& component,
                              TFV_Id id,
                              TFV_Id camera_id,
                              Args... args);
*/
};

/*
    struct ApiRunner {
        void operator()(Api* api) {
            auto timeout = std::chrono::seconds(5);
            auto checkpoint = std::chrono::system_clock::now();
            auto now = std::chrono::system_clock::now();

            while (true) {
                now = std::chrono::system_clock::now();
                if (api->active_components()) {
                    checkpoint = now;
                }
                else {
                    std::cout << "No active components" << std::endl;
                    if ((now - checkpoint) > timeout) {
                        std::cout << "Shutting down" << std::endl;
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            api->stop();
            delete api;
            api = nullptr;
        }
    };
*/

Api& get_api(void);
};
