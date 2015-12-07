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

#include <stdio.h>
#include <unistd.h>   /* sleep (posix) */
#include <time.h>     /* nanosleep (posix) */
#include <sys/time.h> /* nanosleep (posix) */

#include "tinkervision/tinkervision.h"

void callback(int8_t id, TV_ModuleResult result, void* context) {
    printf("Callback for module %d: %d,%d,%d,%d,%s\n", id, result.x, result.y,
           result.width, result.height, result.string);
}

void str_callback(int8_t id, char const* string, void* context) {
    int ctx = *(int*)(context), i;
    uint16_t parameters;
    uint8_t type;
    char name[TV_STRING_SIZE];
    char value[TV_STRING_SIZE];
    int32_t min, max, def;

    printf("String-callback: %d, %s, %d\n", id, string, ctx);
    if (tv_library_parameters_count(string, &parameters) != TV_OK) {
        printf("Error during parameter_count\n");
        return;
    }
    for (i = 0; i < parameters; ++i) {
        if (tv_library_parameter_describe(string, i, name, &type, &min, &max,
                                          &def) != TV_OK) {
            printf("Error during describe_parameter %d\n", i);
            continue;
        }
        if (type == 0) {
            printf("%s/%s: %d [%d, %d]\n", string, name, def, min, max);
        } else {
            printf("%s/%s: %s\n", string, name, value);
        }
    }
}

void colormatch_start(int8_t id, int min_hue, int max_hue) {
    int16_t result = tv_module_start("colormatch", &id);
    printf("Colormatch Id %d Start: %d (%s)\n", id, result,
           tv_result_string(result));
    if (result != TV_OK) {
        return;
    }
    result = tv_module_set_numerical_parameter(id, "min-hue", min_hue);
    printf("Set min-hue: %d (%s)\n", result, tv_result_string(result));
    if (result != TV_OK) {
        return;
    }
    result = tv_module_set_numerical_parameter(id, "max-hue", max_hue);
    printf("Set max-hue: %d (%s)\n", result, tv_result_string(result));
    if (result != TV_OK) {
        return;
    }
    result = tv_module_get_numerical_parameter(id, "min-hue", &min_hue);
    printf("Got min-hue: %d -- %d (%s)\n", min_hue, result,
           tv_result_string(result));
    if (result != TV_OK) {
        return;
    }
    result = tv_module_get_numerical_parameter(id, "max-hue", &max_hue);
    printf("Got max-hue: %d -- %d (%s)\n", max_hue, result,
           tv_result_string(result));
    if (result != TV_OK) {
        return;
    }
}

int main(int argc, char* argv[]) {
    uint16_t width = 1280;
    uint16_t height = 720;
    char string[TV_STRING_SIZE];
    uint32_t period;
    struct timeval before, after;
    double duration;

    int16_t result = tv_valid();
    if (TV_OK != result) {
        printf("Api not valid: %d (%s)\n", result, tv_result_string(result));
        return 0;
    }

    gettimeofday(&before, NULL);
    result = tv_latency_test();
    gettimeofday(&after, NULL);
    duration = difftime(after.tv_sec, before.tv_sec);
    duration += (after.tv_usec - before.tv_usec) / 1000000.0;
    printf("LatencyTest: %d (%s) -> %f sec\n", result, tv_result_string(result),
           duration);

    gettimeofday(&before, NULL);
    result = tv_duration_test(2000); /* milliseconds */
    gettimeofday(&after, NULL);
    duration = difftime(after.tv_sec, before.tv_sec);
    duration += (after.tv_usec - before.tv_usec) / 1000000.0;
    printf("DurationTest: %d (%s) -> %f sec\n", result,
           tv_result_string(result), duration);

    while (TV_RESULT_BUFFERED == (result = tv_get_buffered_result()))
        ;
    gettimeofday(&after, NULL);
    duration = difftime(after.tv_sec, before.tv_sec);
    duration += (after.tv_usec - before.tv_usec) / 1000000.0;
    printf("Got result: %d (%s) -> %f sec\n", result, tv_result_string(result),
           duration);

    result = tv_set_framesize(width, height);
    printf("SetFramesize: %d (%s)\n", result, tv_result_string(result));

    tv_get_user_paths_prefix(string);
    printf("User prefix: %s\n", string);

    tv_get_system_module_load_path(string);
    printf("System module path: %s\n", string);

    result = tv_camera_available();
    printf("CameraAvailable: %d (%s)\n", result, tv_result_string(result));

    result = tv_callback_enable_default(callback);
    printf("Set callback: Code %d (%s)\n", result, tv_result_string(result));

    result = tv_start_idle();
    printf("StartIdle: %d (%s)\n", result, tv_result_string(result));
    sleep(2);

    result = tv_effective_frameperiod(&period);
    printf("Current frameperiod: %d (%d)\n", period, result);

    /*
    period = 500;
    result = tv_request_frameperiod(period);
    printf("Set framerate to %d (%d)\n", period, result);
    sleep(15);

    result = tv_effective_frameperiod(&period);
    printf("Current framerate: %d (%d)\n", period, result);
    return 0;
    */

    width = height = 0;
    result = tv_get_framesize(&width, &height);
    printf("GetResolution: %d (%s)\n", result, tv_result_string(result));
    printf("WxH: %lux%lu\n", (long unsigned)width, (long unsigned)height);
    sleep(1);

    /* new 10-12-2015: change framesize 'while' running */
    width = 640;
    height = 480;

    result = tv_set_framesize(width, height);

    printf("SetFramesize: %d (%s)\n", result, tv_result_string(result));
    sleep(1);

    width = height = 0;
    result = tv_get_framesize(&width, &height);
    printf("GetResolution: %d (%s)\n", result, tv_result_string(result));
    printf("WxH: %lux%lu\n", (long unsigned)width, (long unsigned)height);
    sleep(1);

    /*
      08-05-2015
      Starting a module, quitting the api, starting same id again failed.
    */
    colormatch_start(1, 20, 25);
    /*
       10-29-2015: Deprecated enumeration
    result = tv_module_parameters_enumerate(1, str_callback, &enum_pars);
    printf("Enumerate Parameters registered: %d\n", result);
    */

    sleep(3);

    result = tv_remove_all_modules();
    printf("RemoveAll: %d (%s)\n", result, tv_result_string(result));
    sleep(2);

    result = tv_quit();
    printf("Quit: %d (%s)\n", result, tv_result_string(result));
    sleep(2);

    colormatch_start(1, 20, 25);
    sleep(2);
    /*
       08-05-2015
       That was actually correct behaviour since quit stops the mainloop.
       One could call start...
     */
    result = tv_start();
    printf("Api restarted: %d (%s)\n", result, tv_result_string(result));
    sleep(2);
    /*
       08-05-2015
       ... or better just call stop/start instead of quit, which would
       not delete the active modules: No call to colormatch_start here
       and the idle_process would still be running.
     */
    result = tv_stop();
    printf("Stop: %d (%s)\n", result, tv_result_string(result));
    sleep(2);

    /* \todo: Can't get resolution if inactive... ok? */
    result = tv_get_framesize(&width, &height);
    printf("GetResolution: %d (%s)\n", result, tv_result_string(result));
    sleep(1);

    result = tv_start();
    printf("Api restarted: %d (%s)\n", result, tv_result_string(result));
    sleep(1);

    width = 1280;
    height = 720;

    result = tv_set_framesize(width, height);
    result = tv_get_framesize(&width, &height);
    printf("GetResolution: %d (%s)\n", result, tv_result_string(result));
    printf("SetFramesize: %d (%s)\n", result, tv_result_string(result));
    sleep(1);

    printf("WxH: %lux%lu\n", (long unsigned)width, (long unsigned)height);
    sleep(2);

    return 0;
}
