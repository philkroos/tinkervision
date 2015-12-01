#include <thread>
#include <chrono>
#include <iostream>
#include <thread>
#include <chrono>

#include "tinkervision/tinkervision.h"

void get_args(std::string& sargs) {}

template <typename T, typename... Args>
void get_args(std::string& sargs, T const& t) {
    sargs += std::to_string(t);
}

template <typename T, typename... Args>
void get_args(std::string& sargs, T const& t, Args... args) {
    sargs += std::to_string(t) + ", ";
    get_args(sargs, args...);
}

template <typename... Args>
void check(int16_t result, std::string const& function, Args... args) {
    if (result) {
        std::string sargs;
        get_args(sargs, args...);
        std::cout << "Function failed with " << result << ": " << function
                  << "(" << sargs << ")" << std::endl;
    }
}

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

    auto result = tv_prefer_camera_with_id(c0);
    check(result, "PreferCam", c0);

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
        check(result, "LibParameterDescribe", i);
        if (type == 1) {
            std::cout << "Parameter: " << name << std::endl;
        } else {
            std::cout << "Parameter: " << name << "[" << min << "," << def
                      << "," << max << "]" << std::endl;
        }
    }

    TV_ModuleResult m_result;
    result = tv_module_get_result(module, &m_result);
    std::cout << "Snapshot result: " << m_result.string << " -- "
              << tv_result_string(result) << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    result = tv_prefer_camera_with_id(c1);

    result = tv_module_get_result(module, &m_result);
    std::cout << "Snapshot result: " << m_result.string << " -- "
              << tv_result_string(result) << std::endl;

    tv_quit();
    return 0;
}
