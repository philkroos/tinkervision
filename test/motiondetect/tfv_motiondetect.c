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

static IplImage* image = NULL;

void tfv_motiondetect_callback(TFV_Id id, TFV_Int x_topleft, TFV_Int y_topleft,
                               TFV_Int x_bottomright, TFV_Int y_bottomright,
                               TFV_Context context) {

    auto topleft = CvPoint{x_topleft, y_topleft};
    auto bottomright = CvPoint{x_bottomright, y_bottomright};
    auto thickness = 2;
    auto linetype = CV_AA;
    auto shift = 0;
    auto color = CV_RGB(255, 0, 0);

    cvRectangle(image, topleft, bottomright, color, thickness, linetype, shift);

    cvShowImage("Motion", image);
    cvWaitKey(10);
    cvZero(image);
}

int main(int argc, char* argv[]) {

    // Start an idle process in the api to get access to the frame parameters
    auto result = start_idle();

    if (result) {
        printf("Starting the idle process failed with %d: %s", result,
               result_string(result));
        exit(-1);
    }

    // Just to verify that only one dummy started (monitor the "Destroying ..."
    // messages on quit). This should give an OK here.
    printf("Starting the idle process again: %s\n",
           result_string(start_idle()));

    size_t width, height;
    result = get_resolution(width, height);

    if (result) {
        printf("Retrieving the framesize failed with %d: %s", result,
               result_string(result));
        exit(-1);
    }

    image = cvCreateImage(cvSize(width, height), 8, 3);
    cvNamedWindow("Motion", CV_WINDOW_AUTOSIZE);

    result = motiondetect_start(0, tfv_motiondetect_callback, nullptr);

    if (not result) {
        printf(
            "Motiondetection initialized; this will take a few secs to "
            "adjust\n");
    } else {
        printf("Error %d: %s\n", result, result_string(result));
        cvReleaseImage(&image);
        exit(-1);
    }

    auto dur = 20;
    printf("Detecting motion for %d secs\n", dur);
    sleep(dur);

    cvReleaseImage(&image);
}
