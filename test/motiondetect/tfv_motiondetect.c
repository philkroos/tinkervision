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

#include "tinkervision.h"

static IplImage* image = NULL;

void tfv_motiondetect_callback(TFV_Id id, TFV_Size x_topleft,
                               TFV_Size y_topleft, TFV_Size width,
                               TFV_Size height, TFV_Context context) {

    int thickness = 2;
    int linetype = CV_AA;
    int shift = 0;
    CvPoint topleft;
    CvPoint bottomright;

    topleft.x = x_topleft;
    topleft.y = y_topleft;
    bottomright.x = topleft.x + width;
    bottomright.y = topleft.y + height;

    cvRectangle(image, topleft, bottomright, CV_RGB(255, 0, 0), thickness,
                linetype, shift);

    cvShowImage("Motion", image);
    cvWaitKey(10);
    cvZero(image);
}

int main(int argc, char* argv[]) {

    /* Framedimensions */
    TFV_Size width, height;

    /* Runtime of program */
    int runtime = 20;

    /* Start an idle process in the api to get access to the frame parameters */
    TFV_Result result = start_idle();

    if (result) {
        printf("Starting the idle process failed with %d: %s", result,
               result_string(result));
        exit(-1);
    }

    /* Just to verify that only one dummy started (monitor the "Destroying ..."
       messages on quit). This should give an OK here. */
    printf("Starting the idle process again: %s\n",
           result_string(start_idle()));

    result = get_resolution(&width, &height);

    if (result) {
        printf("Retrieving the framesize failed with %d: %s", result,
               result_string(result));
        exit(-1);
    }

    image = cvCreateImage(cvSize(width, height), 8, 3);
    cvNamedWindow("Motion", CV_WINDOW_AUTOSIZE);

    result = motiondetect_start(0, tfv_motiondetect_callback, NULL);

    if (!result) {
        printf(
            "Motiondetection initialized; this will take a few secs to "
            "adjust\n");
    } else {
        printf("Error %d: %s\n", result, result_string(result));
        cvReleaseImage(&image);
        exit(-1);
    }

    printf("Detecting motion for %d secs\n", runtime);
    sleep(runtime);

    cvReleaseImage(&image);

    return 0;
}
