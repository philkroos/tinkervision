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
#include <unistd.h> /* sleep (posix) */
#include <time.h>   /* nanosleep (posix) */

#include "tinkervision/tinkervision.h"
#define TV_STRING_SIZE 30

void callback(int8_t id, TV_ModuleResult result, void* context) {
    printf("Callback for module %d\n", id);
}

void str_callback(int8_t id, char const* string, void* context) {
    int ctx = *(int*)(context), i;
    uint16_t parameter;
    char name[TV_STRING_SIZE];
    int32_t min, max, def;

    printf("String-callback: %d, %s, %d\n", id, string, ctx);
    if (tv_library_parameter_count(string, &parameter) != TV_OK) {
        printf("Error during parameter_count\n");
        return;
    }
    for (i = 0; i < parameter; ++i) {
        if (tv_library_describe_parameter(string, i, name, &min, &max, &def) !=
            TV_OK) {
            printf("Error during describe_parameter %d\n", i);
            continue;
        }
        printf("%s/%s: %d [%d, %d]\n", string, name, def, min, max);
    }
}

void colormatch_start(int8_t id, int min_hue, int max_hue) {
    int16_t result = tv_module_start("colormatch", &id);
    printf("Colormatch Id %d Start: %d (%s)\n", id, result,
           tv_result_string(result));
    if (result != TV_OK) {
        return;
    }
    result = tv_set_parameter(id, "min-hue", min_hue);
    printf("Set min-hue: %d (%s)\n", result, tv_result_string(result));
    if (result != TV_OK) {
        return;
    }
    result = tv_set_parameter(id, "max-hue", max_hue);
    printf("Set max-hue: %d (%s)\n", result, tv_result_string(result));
    if (result != TV_OK) {
        return;
    }
    result = tv_set_callback(id, callback);
    if (result != TV_OK) {
        printf("Setting the callback failed: %d (%s)\n", result,
               tv_result_string(result));
    }
}

int main(int argc, char* argv[]) {
    uint16_t width = 1280;
    uint16_t height = 720;
    char string[TV_STRING_SIZE];
    int enum_modules = 1;
    /*int enum_pars = 2;*/

    int16_t result = tv_set_framesize(width, height);

    printf("SetFramesize: %d (%s)\n", result, tv_result_string(result));
    sleep(1);

    tv_user_module_load_path(string);
    printf("User module path: %s\n", string);

    tv_system_module_load_path(string);
    printf("System module path: %s\n", string);

    result = tv_camera_available();
    printf("CameraAvailable: %d (%s)\n", result, tv_result_string(result));
    sleep(1);

    result = tv_enumerate_available_modules(str_callback, &enum_modules);
    printf("Enumerate Modules registered: %d\n", result);
    sleep(4);

    result = tv_start_idle();
    printf("StartIdle: %d (%s)\n", result, tv_result_string(result));
    sleep(2);

    width = height = 0;
    result = tv_get_resolution(&width, &height);
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
    result = tv_get_resolution(&width, &height);
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
    result = tv_module_enumerate_parameters(1, str_callback, &enum_pars);
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
    result = tv_get_resolution(&width, &height);
    printf("GetResolution: %d (%s)\n", result, tv_result_string(result));
    sleep(1);

    result = tv_start();
    printf("Api restarted: %d (%s)\n", result, tv_result_string(result));
    sleep(1);

    width = 1280;
    height = 720;

    result = tv_set_framesize(width, height);
    result = tv_get_resolution(&width, &height);
    printf("GetResolution: %d (%s)\n", result, tv_result_string(result));
    printf("SetFramesize: %d (%s)\n", result, tv_result_string(result));
    sleep(1);

    printf("WxH: %lux%lu\n", (long unsigned)width, (long unsigned)height);
    sleep(2);

    return 0;
}
