#include <thread>
#include <chrono>
#include <iostream>
#include <thread>
#include <chrono>

#include "tinkervision/tinkervision.h"
#include "../check_result.hh"

struct ScopeTimer {
    using Clock = std::chrono::steady_clock;
    std::string id_;
    Clock::time_point begin{Clock::now()};

    ScopeTimer(char const* id) : id_(id) {}
    ~ScopeTimer(void) {
        std::cout << "Scope " << id_ << " took "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         Clock::now() - begin).count() << "ms" << std::endl;
    }
};

int main(void) {

    int8_t module;
    auto result = tv_module_start("blocking", &module);
    check(result, "ModuleStart");

    // wait for blocking to be executing surely
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // no-problem calls
    uint16_t count;
    {
        ScopeTimer bt("parameter-count");
        result = tv_library_parameters_count("blocking", &count);
    }
    check(result, "ParametersCount");
    std::cout << "Parameters: " << count << std::endl;

    char name[TV_STRING_SIZE];
    {
        ScopeTimer bt("get-name");
        result = tv_module_get_name(module, name);
    }
    check(result, "GetName");
    std::cout << "Name: " << name << std::endl;

    // Add another module

    int8_t module_color;
    {
        ScopeTimer st("module-start");
        result = tv_module_start("colormatch", &module_color);
    }
    check(result, "ModuleStart");
    std::cout << "Added module colormatch" << std::endl;

    // wait for blocking to be executing surely
    std::this_thread::sleep_for(std::chrono::seconds(2));
    {
        ScopeTimer bt("module-remove");
        while (tv_module_remove(module_color) > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    check(result, "ModuleRemove");
    std::cout << "Removed colormatch? - " << tv_result_string(result)
              << std::endl;

    {
        ScopeTimer bt("quit");
        tv_quit();
    }

    return 0;

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
}
