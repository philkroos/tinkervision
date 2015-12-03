#include <thread>
#include <chrono>

#include "tinkervision/tinkervision.h"
#include "../check_result.hh"

int main(void) {
    int8_t id;

    auto result = tv_prefer_camera_with_id(0);
    check(result, "PreferCamera", 0);

    result = tv_module_start("gesture", &id);
    check(result, "ModuleStart", "Gesture");

    result = tv_module_set_numerical_parameter(id, "fg-threshold", 30);
    check(result, "SetParameter", "fg-threshold", 50);

    std::this_thread::sleep_for(std::chrono::seconds(20));
    tv_quit();
    return 0;
}
