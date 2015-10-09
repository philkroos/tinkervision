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
#include <unistd.h>
#include <time.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "tinkervision/tinkervision.h"

static IplImage* image = NULL;

void tv_callback(TV_Id id, TV_ModuleResult result, TV_Context context) {

    int thickness = 2;
    int linetype = CV_AA;
    int shift = 0;
    CvPoint topleft;
    CvPoint bottomright;

    topleft.x = result.x;
    topleft.y = result.y;
    bottomright.x = result.x + result.width;
    bottomright.y = result.y + result.height;

    cvRectangle(image, topleft, bottomright, CV_RGB(255, 0, 0), thickness,
                linetype, shift);

    cvShowImage("Motion", image);
    cvWaitKey(10);
    cvZero(image);
}

int main(int argc, char* argv[]) {

    /* Framedimensions */
    TV_Size width, height;

    /* Runtime of program */
    int runtime = 20;

    TV_Id module_id = 0;

    /* Start an idle process in the api to get access to the frame parameters */
    TV_Result result = tv_start_idle();

    if (result) {
        printf("Starting the idle process failed with %d: %s\n", result,
               tv_result_string(result));
        exit(-1);
    }

    result = tv_get_resolution(&width, &height);

    if (result) {
        printf("Retrieving the framesize failed with %d: %s\n", result,
               tv_result_string(result));
        exit(-1);
    }

    image = cvCreateImage(cvSize(width, height), 8, 3);
    cvNamedWindow("Motion", CV_WINDOW_AUTOSIZE);

    result = tv_module_start("motiondetect", &module_id);

    if (result != TV_OK) {
        printf("Error - could not start the motiondetector: %d (%s)\n", result,
               tv_result_string(result));
    }

    result = tv_set_callback(module_id, tv_callback);

    if (!result) {
        printf(
            "Motiondetection initialized; this will take a few secs to "
            "adjust\n");
    } else {
        printf("Error %d: %s\n", result, tv_result_string(result));
        cvReleaseImage(&image);
        exit(-1);
    }

    printf("Detecting motion for %d secs\n", runtime);
    sleep(runtime);

    cvReleaseImage(&image);

    return 0;
}
