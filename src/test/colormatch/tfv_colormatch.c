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

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "tinkervision/tinkervision.h"

static IplImage* image = NULL;

void callback(TV_Id id, TV_ModuleResult result, TV_Context context) {
    CvPoint center;

    center.x = result.x;
    center.y = result.y;

    printf("Id %d: Located at %d/%d\n", id, center.x, center.y);

    cvCircle(image, center, 5, CV_RGB(255, 0, 0), 2, CV_AA, 0);
    cvShowImage("Result", image);
    cvWaitKey(10);
}

void set(TV_Long* min, TV_Long* max, TV_Byte value, TV_Byte range) {
    *min = value >= range ? value - range : *max - range + value;
    *max = value < (*max - range) ? value + range : range - value;
}

int main(int argc, char* argv[]) {
    TV_Id id = 0;
    TV_Byte hue, saturation, value; /* commandline */
    TV_Long min_hue;
    TV_Long max_hue = 180;
    TV_Long min_value;
    TV_Long max_value = 255;
    TV_Long min_saturation;
    TV_Long max_saturation = 255;
    TV_Byte range;
    TV_Size width, height; /* framesize */
    TV_Result result = TV_INTERNAL_ERROR;

    if (argc < 5) {
        printf(
            "Usage: %s h s v range, where\n"
            "h is [0-180), s and v [0-255], range is an allowed "
            "deviation\n",
            argv[0]);
        return -1;
    }

    hue = (TV_Byte)atoi(argv[1]);
    saturation = (TV_Byte)atoi(argv[2]);
    value = (TV_Byte)atoi(argv[3]);
    range = (TV_Byte)atoi(argv[4]);

    set(&min_hue, &max_hue, hue, range);
    set(&min_value, &max_value, value, range);
    set(&min_saturation, &max_saturation, saturation, range);

    printf("Using H-S-V: [%d,%d]-[%d,%d]-[%d,%d]\n", min_hue, max_hue,
           min_value, max_value, min_saturation, max_saturation);

    result = tv_camera_available();
    if (result != 0) {
        printf("Requested camera not available: %s\n",
               tv_result_string(result));
        exit(-1);
    }

    sleep(1);

    result = tv_module_start("colormatch", &id);

    printf("Configured module id %d: Code %d (%s)\n", id, result,
           tv_result_string(result));

    result = tv_enable_default_callback(callback);
    printf("Set callback: Code %d (%s)\n", result, tv_result_string(result));

    result = tv_set_parameter(id, "min-hue", min_hue);
    printf("Set min-hue: Code %d (%s)\n", result, tv_result_string(result));
    result = tv_get_parameter(id, "min-hue", &min_hue);
    printf("%d Code %d (%s)\n", min_hue, result, tv_result_string(result));

    result = tv_set_parameter(id, "max-hue", max_hue);
    printf("Set max-hue: Code %d (%s)\n", result, tv_result_string(result));
    result = tv_get_parameter(id, "max-hue", &min_hue);
    printf("%d Code %d (%s)\n", max_hue, result, tv_result_string(result));

    result = tv_set_parameter(id, "min-value", min_value);
    printf("Set min-value: Code %d (%s)\n", result, tv_result_string(result));
    result = tv_get_parameter(id, "min-value", &min_hue);
    printf("%d Code %d (%s)\n", min_value, result, tv_result_string(result));

    result = tv_set_parameter(id, "max-value", max_value);
    printf("Set max-value: Code %d (%s)\n", result, tv_result_string(result));
    result = tv_get_parameter(id, "max-value", &min_hue);
    printf("%d Code %d (%s)\n", max_value, result, tv_result_string(result));

    result = tv_set_parameter(id, "min-saturation", min_saturation);
    printf("Set min-sat: Code %d (%s)\n", result, tv_result_string(result));
    result = tv_get_parameter(id, "min-saturation", &min_saturation);
    printf("%d Code %d (%s)\n", min_saturation, result,
           tv_result_string(result));

    result = tv_set_parameter(id, "max-saturation", max_saturation);
    printf("Set max-sat: Code %d (%s)\n", result, tv_result_string(result));
    result = tv_get_parameter(id, "max-saturation", &min_saturation);
    printf("%d Code %d (%s)\n", max_saturation, result,
           tv_result_string(result));

    sleep(2);

    tv_get_resolution(&width, &height);
    printf("WxH: %dx%d (Code %d: %s)\n", width, height, result,
           tv_result_string(result));
    image = cvCreateImage(cvSize(width, height), 8, 3);
    cvZero(image);
    cvNamedWindow("Result", CV_WINDOW_AUTOSIZE);

    sleep(10);

    result = tv_module_remove(id);
    printf("Removed module %d: Code %d (%s)\n", id, result,
           tv_result_string(result));

    cvReleaseImage(&image);

    /* Stopping manually is not necessary but can be used to stop active
       resources if a client app should have crashed. */
    tv_quit();

    sleep(2);

    return 0;
}
