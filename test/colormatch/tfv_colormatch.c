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

#include "tinkervision.h"

static IplImage* image = NULL;

void tfcv_callback_id0(TFV_Id id, TFV_Size x, TFV_Size y, TFV_Context context) {
    CvPoint center;

    center.x = x;
    center.y = y;

    printf("Id %d: Feature at %d/%d\n", id, center.x, center.y);

    cvCircle(image, center, 5, CV_RGB(255, 0, 0), 2, CV_AA, 0);
    cvShowImage("Result", image);
    cvWaitKey(10);
}

int main(int argc, char* argv[]) {
    TFV_Id id = 0;
    TFV_Byte hue;
    TFV_Byte min_hue;
    TFV_Byte max_hue;
    TFV_Byte range = 3;
    struct timespec time = {0};
    int i;
    TFV_Size width, height; /* framesize */
    TFV_Result result = TFV_INTERNAL_ERROR;

    if (argc < 2) {
        printf("Usage: %s min-hue, where min-hue is an integer [0-180)\n",
               argv[0]);
        return -1;
    }

    hue = (TFV_Byte)atoi(argv[1]);
    min_hue = hue >= range ? hue - range : 180 - range + hue;
    max_hue = hue < (180 - range) ? hue + range : range - hue;
    printf("Using min-hue: %d and max-hue: %d\n", min_hue, max_hue);

    result = camera_available();
    if (result != 0) {
        printf("Requested camera not available: %s\n", result_string(result));
        exit(-1);
    }

    sleep(1);

    result = colormatch_start(id, min_hue, max_hue, tfcv_callback_id0, NULL);

    printf("Configured module id %d: Code %d (%s)\n", id, result,
           result_string(result));
    sleep(1);

    get_resolution(&width, &height);
    image = cvCreateImage(cvSize(width, height), 8, 3);
    cvZero(image);
    cvNamedWindow("Result", CV_WINDOW_AUTOSIZE);

    time.tv_sec = 0;
    time.tv_nsec = 500000000L;
    for (i = 0; i < 40; i++) {
        nanosleep(&time, (struct timespec*)NULL);
    }
    cvReleaseImage(&image);

    /* Stopping last module */
    result = colormatch_stop(id);
    printf("Stopped module %d: Code %d (%s)\n", id, result,
           result_string(result));

    /* Stopping manually is not necessary but can be used to stop active
       resources if a client app should have crashed. */
    quit();

    sleep(2);

    return 0;
}
