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

void invalid_callback(TFV_Id id, TFV_Size x, TFV_Context context) {
    (void)id;
    (void)x;
    (void)context;
}
void callback(TFV_Id id, TFV_Size x, TFV_Size y, TFV_Context context) {
    CvPoint center;

    center.x = x;
    center.y = y;

    printf("Id %d: Located at %d/%d\n", id, center.x, center.y);

    cvCircle(image, center, 5, CV_RGB(255, 0, 0), 2, CV_AA, 0);
    cvShowImage("Result", image);
    cvWaitKey(10);
}

void set(TFV_Word* min, TFV_Word* max, TFV_Byte value, TFV_Byte range) {
    *min = value >= range ? value - range : *max - range + value;
    *max = value < (*max - range) ? value + range : range - value;
}

int main(int argc, char* argv[]) {
    TFV_Id id = 0;
    TFV_Byte hue, saturation, value; /* commandline */
    TFV_Word min_hue;
    TFV_Word max_hue = 180;
    TFV_Word min_value;
    TFV_Word max_value = 255;
    TFV_Word min_saturation;
    TFV_Word max_saturation = 255;
    TFV_Byte range;
    TFV_Size width, height; /* framesize */
    TFV_Result result = TFV_INTERNAL_ERROR;

    if (argc < 5) {
        printf(
            "Usage: %s h s v range, where\n"
            "h is [0-180), s and v [0-255], range is an allowed "
            "deviation\n",
            argv[0]);
        return -1;
    }

    hue = (TFV_Byte)atoi(argv[1]);
    saturation = (TFV_Byte)atoi(argv[2]);
    value = (TFV_Byte)atoi(argv[3]);
    range = (TFV_Byte)atoi(argv[4]);

    set(&min_hue, &max_hue, hue, range);
    set(&min_value, &max_value, value, range);
    set(&min_saturation, &max_saturation, saturation, range);

    printf("Using H-S-V: [%d,%d]-[%d,%d]-[%d,%d]\n", min_hue, max_hue,
           min_value, max_value, min_saturation, max_saturation);

    result = camera_available();
    if (result != 0) {
        printf("Requested camera not available: %s\n", result_string(result));
        exit(-1);
    }

    sleep(1);

    result = module_start("colormatch", id);

    printf("Configured module id %d: Code %d (%s)\n", id, result,
           result_string(result));

    result = set_value_callback(id, invalid_callback);
    printf("Set value callback: Code %d (%s)\n", result, result_string(result));

    result = set_point_callback(id, callback);
    printf("Set point callback: Code %d (%s)\n", result, result_string(result));

    result = set_parameter(id, "min-hue", min_hue);
    printf("Set min-hue: Code %d (%s)\n", result, result_string(result));
    result = get_parameter(id, "min-hue", &min_hue);
    printf("%d Code %d (%s)\n", min_hue, result, result_string(result));

    result = set_parameter(id, "max-hue", max_hue);
    printf("Set max-hue: Code %d (%s)\n", result, result_string(result));
    result = get_parameter(id, "max-hue", &min_hue);
    printf("%d Code %d (%s)\n", max_hue, result, result_string(result));

    result = set_parameter(id, "min-value", min_value);
    printf("Set min-value: Code %d (%s)\n", result, result_string(result));
    result = get_parameter(id, "min-value", &min_hue);
    printf("%d Code %d (%s)\n", min_value, result, result_string(result));

    result = set_parameter(id, "max-value", max_value);
    printf("Set max-value: Code %d (%s)\n", result, result_string(result));
    result = get_parameter(id, "max-value", &min_hue);
    printf("%d Code %d (%s)\n", max_value, result, result_string(result));

    result = set_parameter(id, "min-saturation", min_saturation);
    printf("Set min-sat: Code %d (%s)\n", result, result_string(result));
    result = get_parameter(id, "min-saturation", &min_saturation);
    printf("%d Code %d (%s)\n", min_saturation, result, result_string(result));

    result = set_parameter(id, "max-saturation", max_saturation);
    printf("Set max-sat: Code %d (%s)\n", result, result_string(result));
    result = get_parameter(id, "max-saturation", &min_saturation);
    printf("%d Code %d (%s)\n", max_saturation, result, result_string(result));

    sleep(2);

    get_resolution(&width, &height);
    image = cvCreateImage(cvSize(width, height), 8, 3);
    cvZero(image);
    cvNamedWindow("Result", CV_WINDOW_AUTOSIZE);

    sleep(10);

    result = module_remove(id);
    printf("Removed module %d: Code %d (%s)\n", id, result,
           result_string(result));

    cvReleaseImage(&image);

    /* Stopping manually is not necessary but can be used to stop active
       resources if a client app should have crashed. */
    quit();

    sleep(2);

    return 0;
}
