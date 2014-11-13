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
#include <unistd.h>  // sleep (posix)

#include "tinkervision.h"

void tfcv_callback_id0(TFV_Id id, TFV_Int* x, TFV_Int* y, TFV_Int* width,
                       TFV_Int* height, TFV_Byte count, TFV_Context context) {
    printf("Callback for id %d with %d features\n", id, count);
}

int main(int argc, char* argv[]) {
    TFV_Id id = 0;
    TFV_Id cam = 0;
    TFV_Id invalid_id = 100;
    TFV_Byte min_hue = 200;
    TFV_Byte max_hue = 250;

    test_();
    int test = 0;
    // Test 0: Camera should be available
    TFV_Result result = camera_available(cam);
    printf("%d: Requested camera %d, code %d (%s)\n", test++, cam, result,
           result_string(result));

    sleep(1);

    // Test 1: Camera should not be available
    result = camera_available(invalid_id);
    printf("%d: Requested camera %d, code %d (%s)\n", test++, invalid_id,
           result, result_string(result));

    sleep(1);

    // Test 2: configuration of two new features
    result =
        colortracking_start(id, cam, min_hue, max_hue, tfcv_callback_id0, NULL);

    printf("%d: Configured feature id %d: Code %d (%s)\n", test++, id, result,
           result_string(result));

    sleep(1);

    id += 1;
    result =
        colortracking_start(id, cam, min_hue, max_hue, tfcv_callback_id0, NULL);

    printf("%d: Configured feature id %d: Code %d (%s)\n", test++, id, result,
           result_string(result));

    sleep(1);

    // Test 3: invalid configuration of new feature (min > max-hue)
    result = colortracking_start(id + 1, cam, 100, 0, tfcv_callback_id0, NULL);

    printf("%d: Configuring invalid feature id %d: Code %d (%s)\n", test++,
           invalid_id, result, result_string(result));

    sleep(1);

    // Test 4: invalid configuration (missing callback)
    result = colortracking_start(id + 1, cam, 100, 0, tfcv_callback_id0, NULL);

    printf("%d: Configuring invalid feature id %d: Code %d (%s)\n", test++,
           invalid_id, result, result_string(result));

    sleep(1);

    // Test 5: reconfiguration of a feature
    result =
        colortracking_start(id, cam, min_hue, max_hue, tfcv_callback_id0, NULL);

    printf("%d: Re-Configured feature id %d: Code %d (%s)\n", test++, id,
           result, result_string(result));

    sleep(1);

    // Test 6: stop and restart of a feature
    result = colortracking_stop(id);
    printf("%d: Stopped configured feature id %d: Code %d (%s)\n", test++, id,
           result, result_string(result));

    sleep(1);

    // Test 7:
    result = colortracking_restart(id);
    printf("%d: Restarted configured feature id %d: Code %d (%s)\n", test++, id,
           result, result_string(result));

    sleep(1);

    // Test 8: request for configuration details
    cam = 1;
    min_hue = 30;
    min_hue = 30;
    result = colortracking_get(id, &cam, &min_hue, &max_hue);
    printf(
        "%d: Got configured feature id %d: Code %d (%s)\n "
        "Cam-Id: %d, min-hue: %d, max-hue: %d\n",
        test++, id, result, result_string(result), cam, min_hue, max_hue);

    // Test 9: request for details of invalid feature
    result = colortracking_get(invalid_id, &cam, &min_hue, &max_hue);
    printf(
        "%d: Got configured feature id %d: Code %d (%s)\n "
        "Cam-Id: %d, min-hue: %d, max-hue: %d\n",
        test++, invalid_id, result, result_string(result), cam, min_hue,
        max_hue);

    // Test 10: Camera should still be available
    cam = 0;
    result = camera_available(cam);
    printf("%d: Requested camera %d, code %d (%s)\n", test++, cam, result,
           result_string(result));

    // Test 11: Stopping invalid feature
    result = colortracking_stop(invalid_id);
    printf("%d: Stopped invalid feature %d: Code %d (%s)\n", test++, id, result,
           result_string(result));

    // Stopping all is preferred; else libv4l2 might throw errors

    while (id) {
        result = colortracking_stop(id);
        printf("%d: Stopped feature %d: Code %d (%s)\n", test++, id--, result,
               result_string(result));
    }
    result = colortracking_stop(id);
    printf("%d: Stopped feature %d: Code %d (%s)\n", test++, id, result,
           result_string(result));

    stop_api();

    sleep(10);
}
