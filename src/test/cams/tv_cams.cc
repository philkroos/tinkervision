#include <thread>
#include <chrono>
#include <iostream>
#include <thread>
#include <chrono>

#include "tinkervision/tinkervision.h"
#include "../check_result.hh"

int main(void) {
    uint8_t c0 = 0, c1, max_cam = 10;

    while (TV_OK != tv_camera_id_available(c0) and c0 < max_cam) c0++;
    c1 = c0 + 1;
    while (TV_OK != tv_camera_id_available(c1) and c1 < max_cam) c1++;

    if (c1 >= max_cam) {
        std::cout << "This needs two cams to make sense" << std::endl;
        return 0;
    }
    std::cout << "Using cameras " << int(c0) << " and " << int(c1) << std::endl;

    auto result = tv_prefer_camera_with_id(c1);
    check(result, "PreferCam", c1);

    int8_t module;
    result = tv_module_start("snapshot", &module);
    check(result, "ModuleStart");

    uint16_t count;
    result = tv_library_parameters_count("snapshot", &count);
    check(result, "ParametersCount");

    char name[TV_STRING_SIZE];
    uint8_t type;
    int32_t min, max, def;
    for (size_t i = 0; i < count; ++i) {
        result = tv_library_parameter_describe("snapshot", i, name, &type, &min,
                                               &max, &def);
        check(result, "LibParameterDescribe", "snapshot", i);
        if (type == 1) {
            std::cout << "Parameter: " << name << std::endl;
        } else {
            std::cout << "Parameter: " << name << "[" << min << "," << def
                      << "," << max << "]" << std::endl;
        }
    }

    result = tv_module_set_string_parameter(module, "format", "tiff");
    check(result, "SetStringParameter:Format:tiff");

    std::this_thread::sleep_for(std::chrono::seconds(4));
    TV_ModuleResult m_result;
    while (TV_OK != tv_module_get_result(module, &m_result))
        ;
    std::cout << "Snapshot result: " << m_result.string << " -- "
              << tv_result_string(result) << std::endl;

    result = tv_prefer_camera_with_id(c0);
    check(result, "PreferCam", c0);
    std::this_thread::sleep_for(std::chrono::seconds(4));

    /* Haertetest */
    while (TV_OK != tv_prefer_camera_with_id(c1))
        ;
    while (TV_OK != tv_prefer_camera_with_id(c0))
        ;
    while (TV_OK != tv_prefer_camera_with_id(c1))
        ;
    while (TV_OK != tv_prefer_camera_with_id(c0))
        ;
    while (TV_OK != tv_prefer_camera_with_id(c1))
        ;
    while (TV_OK != tv_prefer_camera_with_id(c0))
        ;
    while (TV_OK != tv_prefer_camera_with_id(c1))
        ;
    while (TV_OK != tv_prefer_camera_with_id(c0))
        ;

    while (TV_OK != tv_module_get_result(module, &m_result))
        ;
    std::cout << "Snapshot result: " << m_result.string << " -- "
              << tv_result_string(result) << std::endl;

    tv_quit();
    return 0;
}
