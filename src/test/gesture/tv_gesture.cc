#include <thread>
#include <chrono>

#include "tinkervision/tinkervision.h"
#include "../check_result.hh"

int main(void) {
    int8_t id;
    uint16_t handsize(250);
    uint8_t threshold(50);

    (void)checked(tv_prefer_camera_with_id, "PreferCam", 1);

    auto result = tv_module_start("gesture", &id);
    if (not check(result, "ModuleStart", "gesture") or
        not checked(tv_module_set_numerical_parameter, "SetParameter", id,
                    "fg-threshold", threshold) or
        not checked(tv_module_set_numerical_parameter, "SetParameter", id,
                    "min-hand-size", handsize)) {
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(40));
    tv_quit();
    return 0;
}
