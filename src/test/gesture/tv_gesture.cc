#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>

#include "tinkervision/tinkervision.h"
#include "../check_result.hh"

int main(int argc, char** argv) {
    // minimal connected area to be considered a hand
    uint16_t handsize(250);

    // Minimal difference for foreground detection
    uint8_t threshold(30);

    // Frames collected before foreground detection starts
    uint8_t history(30);

    // Preferred camera
    uint8_t cam(0);

    std::vector<std::string> args{"--history", "--threshold", "--handsize",
                                  "--cam"};
    for (int i = 1; i < argc; ++i) {
        std::string const arg = argv[i];
        if (args.cend() == std::find(args.cbegin(), args.cend(), arg)) {
            std::cout << "Unsupported arg " << arg << std::endl;
            return -1;
        }
        if (i >= argc - 1) {
            std::cout << "Missing number for " << arg << std::endl;
            return -1;
        }
        if (arg == "--cam") {
            cam = atoi(argv[++i]);
        } else if (arg == "--threshold") {
            threshold = atoi(argv[++i]);
        } else if (arg == "--handsize") {
            handsize = atoi(argv[++i]);
        } else if (arg == "--history") {
            history = atoi(argv[++i]);
        }
    }

    std::cout << "-- Camera: " << std::to_string(cam) << std::endl;
    std::cout << "-- BG History: " << std::to_string(history) << std::endl;
    std::cout << "-- FG Threshold: " << std::to_string(threshold) << std::endl;
    std::cout << "-- Minimal Handsize: " << handsize << std::endl;

    (void)checked(tv_prefer_camera_with_id, "PreferCam", cam);

    int8_t id;
    auto result = tv_module_start("gesture", &id);
    if (not check(result, "ModuleStart", "gesture") or
        not checked(tv_module_set_numerical_parameter, "SetParameter", id,
                    "fg-threshold", threshold) or
        not checked(tv_module_set_numerical_parameter, "SetParameter", id,
                    "bg-history", history) or
        not checked(tv_module_set_numerical_parameter, "SetParameter", id,
                    "min-hand-size", handsize)) {
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(120));
    tv_quit();
    return 0;
}
