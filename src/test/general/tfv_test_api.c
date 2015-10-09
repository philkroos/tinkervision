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

void callback(TFV_Id id, TFV_ModuleResult result, TFV_Context context) {
    printf("Callback for module %d\n", id);
}

void str_callback(TFV_Id id, TFV_String string, TFV_Context context) {
    int ctx = *(int*)(context);
    printf("String-callback: %d, %s, %d\n", id, string, ctx);
}

void colormatch_start(TFV_Id id, int min_hue, int max_hue) {
    TFV_Result result = module_start("colormatch", &id);
    printf("Colormatch Id %d Start: %d (%s)\n", id, result,
           result_string(result));
    if (result != TFV_OK) {
        return;
    }
    result = set_parameter(id, "min-hue", min_hue);
    printf("Set min-hue: %d (%s)\n", result, result_string(result));
    if (result != TFV_OK) {
        return;
    }
    result = set_parameter(id, "max-hue", max_hue);
    printf("Set max-hue: %d (%s)\n", result, result_string(result));
    if (result != TFV_OK) {
        return;
    }
    result = set_callback(id, callback);
    if (result != TFV_OK) {
        printf("Setting the callback failed: %d (%s)\n", result,
               result_string(result));
    }
}

int main(int argc, char* argv[]) {
    TFV_Size width = 640;
    TFV_Size height = 480;
    int enum_modules = 1;
    int enum_pars = 2;

    TFV_Result result = preselect_framesize(width, height);

    printf("PreselectFramesize: %d (%s)\n", result, result_string(result));
    sleep(1);

    result = camera_available();
    printf("CameraAvailable: %d (%s)\n", result, result_string(result));
    sleep(1);

    result = enumerate_available_modules(str_callback, &enum_modules);
    printf("Enumerate Modules registered: %d\n", result);
    sleep(4);

    result = start_idle();
    printf("StartIdle: %d (%s)\n", result, result_string(result));
    sleep(2);

    width = height = 0;
    result = get_resolution(&width, &height);
    printf("GetResolution: %d (%s)\n", result, result_string(result));
    printf("WxH: %lux%lu\n", (long unsigned)width, (long unsigned)height);
    sleep(1);

    /*
      08-05-2015
      Starting a module, quitting the api, starting same id again failed.
    */
    colormatch_start(1, 20, 25);
    result = module_enumerate_parameters(1, str_callback, &enum_pars);
    printf("Enumerate Parameters registered: %d\n", result);

    sleep(3);

    result = quit();
    printf("Quit: %d (%s)\n", result, result_string(result));
    sleep(2);

    colormatch_start(1, 20, 25);
    sleep(2);
    /*
       08-05-2015
       That was actually correct behaviour since quit stops the mainloop.
       One could call start...
     */
    result = start();
    printf("Api restarted: %d (%s)\n", result, result_string(result));
    sleep(2);
    /*
       08-05-2015
       ... or better just call stop/start instead of quit, which would
       not delete the active modules: No call to colormatch_start here
       and the idle_process would still be running.
     */
    result = stop();
    printf("Stop: %d (%s)\n", result, result_string(result));
    sleep(2);

    result = start();
    printf("Api restarted: %d (%s)\n", result, result_string(result));
    sleep(2);

    return 0;
}
