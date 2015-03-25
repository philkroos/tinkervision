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
#include <time.h>    // nanosleep (posix)

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "tinkervision.h"

static int draw = 0;
static IplImage* image = NULL;

void tfcv_callback_id0(TFV_Id id, TFV_Int x, TFV_Int y, TFV_Context context) {
    if (!draw) {
        return;
    }

    CvPoint center;
    center.x = x;
    center.y = y;

    // printf("Id %d: Drawing at %d/%d\n", id, center.x, center.y);

    cvCircle(image, center, 5, CV_RGB(255, 0, 0), 2, CV_AA, 0);
    cvShowImage("Result", image);
    cvWaitKey(10);
}

int main(int argc, char* argv[]) {
    TFV_Id id = 0;
    TFV_Id invalid_id = 100;
    TFV_Byte min_hue = 18;
    TFV_Byte max_hue = 25;

    // Performing some tests. The +/-/# signs in front of the output mean:
    // + should have returned OK
    // - should have returned an error
    // # returns different, depending on some external context

    int test = 0;
    // Test 0: Camera should be available
    TFV_Result result = TFV_INTERNAL_ERROR;
    result = camera_available();
    printf("+ %d: Requested camera, code %d (%s)\n", test++, result,
           result_string(result));

    sleep(1);

    // Test 2: configuration of two new features
    result = colortracking_start(id, min_hue, max_hue, tfcv_callback_id0, NULL);

    printf("+ %d: Configured feature id %d: Code %d (%s)\n", test++, id, result,
           result_string(result));
    sleep(1);

    id += 1;
    result = colortracking_start(id, min_hue, max_hue, tfcv_callback_id0, NULL);

    printf("+ %d: Configured feature id %d: Code %d (%s)\n", test++, id, result,
           result_string(result));

    sleep(5);

    // Test 3: Check CPU-load while this is running!
    printf("Setting execution latency to max, expect high CPU-usage!\n");
    (void)set_execution_latency(0);
    sleep(15);

    printf("Setting execution latency to 100 (ms), CPU should drop.\n");
    (void)set_execution_latency(100);
    sleep(15);

    // Test 4: invalid configuration (missing callback)
    result = colortracking_start(id + 1, 100, 0, NULL, NULL);

    printf("- %d: Configuring invalid feature id %d: Code %d (%s)\n", test++,
           id + 1, result, result_string(result));

    sleep(1);

    // Test 5: reconfiguration of a feature
    result = colortracking_start(id, min_hue, max_hue, tfcv_callback_id0, NULL);

    printf("+ %d: Re-Configured feature id %d: Code %d (%s)\n", test++, id,
           result, result_string(result));

    sleep(1);

    // Test 6: stop and restart of a feature
    id = 0;
    printf("Stopping id %d...\n", id);
    result = colortracking_stop(id);
    printf("+ %d: Stopped configured feature id %d: Code %d (%s)\n", test++, id,
           result, result_string(result));

    sleep(3);
    id = 1;
    printf("Stopping id %d...\n", id);
    result = colortracking_stop(id);
    printf("+ %d: Stopped configured feature id %d: Code %d (%s)\n", test++, id,
           result, result_string(result));

    sleep(3);  // cam down?

    printf("Restarting id %d...\n", id);
    result = colortracking_restart(id);
    printf("+ %d: Restarted configured feature id %d: Code %d (%s)\n", test++,
           id, result, result_string(result));

    sleep(1);

    // Test 7: Restart running feature
    result = colortracking_restart(id);
    printf("+ %d: Restarted configured feature id %d: Code %d (%s)\n", test++,
           id, result, result_string(result));

    sleep(1);

    // Test 8: Second camera - ok if attached (and no usb-bus error...),
    // highgui-error if not attached.
    // int id2 = 40;
    // result = colortracking_start(id2, cam2, min_hue, max_hue,
    // tfcv_callback_id0,
    //                            NULL);
    //
    // ## 03-24-2015: Removed the option to have several cams.

    // Test 9: request for configuration details
    min_hue = -1;
    min_hue = -1;
    result = colortracking_get(id, &min_hue, &max_hue);
    printf(
        "+ %d: Got configured feature id %d: Code %d (%s)\n "
        "min-hue: %d, max-hue: %d\n",
        test++, id, result, result_string(result), min_hue, max_hue);

    // Test 10: request for details of invalid feature
    min_hue = -1;
    min_hue = -1;
    result = colortracking_get(invalid_id, &min_hue, &max_hue);
    printf(
        "- %d: Got configured feature id %d: Code %d (%s)\n "
        "min-hue: %d, max-hue: %d\n",
        test++, invalid_id, result, result_string(result), min_hue, max_hue);

    // Test 11: Try pausing and resuming execution; watch the camera activity.
    result = stop();
    printf("+ %d: Api stopped; cameras should be down for 5 seconds: %d (%s)\n",
           test++, result, result_string(result));
    sleep(5);

    // Camera should still be available
    result = camera_available();
    printf("+ %d: Requested camera, code %d (%s)\n", test, result,
           result_string(result));

    sleep(1);
    result = start();
    printf("+ %d: Resumed execution, cam should come up: %d (%s)\n", test,
           result, result_string(result));

    sleep(5);

    // Failing: Reliance test: remove cam during execution!

    // ### 03-24-2015: To be checked again, should work now.

    // This fails since v4l assigns a different id once the cam is reattached!
    /* printf( */
    /*     "# %d: Remove the camera during execution and reattach it! (20 " */
    /*     "seconds)\n", */
    /*     test); */
    /* sleep(20); */
    /*

    // Test 12: Stopping invalid feature
    result = colortracking_stop(invalid_id);
    printf("# %d: Stopped invalid feature %d: Code %d (%s)\n", test++,
           invalid_id, result, result_string(result));

    // Test 13: 5 users per cam are configured

    // ### 03-24-2015: Removed that limit

    cam = 0;
    min_hue = 0;
    while (id < 8) {
        id += 1;
        result = colortracking_start(id, cam, min_hue, max_hue,
                                     tfcv_callback_id0, NULL);

        printf("+ %d: Configured feature id %d: Code %d (%s)\n", test++, id,
               result, result_string(result));
    }

    sleep(1);
    // Stopping all is preferred; else libv4l2 might throw errors

    while (id) {
        result = colortracking_stop(id);
        printf("+ %d: Stopped feature %d: Code %d (%s)\n", test++, id--,
   result,
               result_string(result));
    }

    result = colortracking_stop(id2);
    printf("+ %d: Stopped feature %d: Code %d (%s)\n", test++, id2, result,
           result_string(result));

    sleep(2);

    printf(
        "--- All features stopped but id 0; now showing results of tracking "
        "---\n");
    image = cvCreateImage(cvSize(640, 480), 8, 3);
    cvZero(image);
    cvNamedWindow("Result", CV_WINDOW_AUTOSIZE);
    draw = 1;
    struct timespec time = {0};
    time.tv_sec = 0;
    time.tv_nsec = 500000000L;
    for (int i = 0; i < 40; i++) {
        nanosleep(&time, (struct timespec*)NULL);
    }
    cvReleaseImage(&image);

    // Stopping last feature
    result = colortracking_stop(id);
    printf("+ %d: Stopped feature %d: Code %d (%s)\n", test++, id, result,
           result_string(result));
    */
    // Stopping manually is not necessary but can be used to stop active
    // resources
    // if a client app should have crashed.
    quit();

    sleep(4);
}
