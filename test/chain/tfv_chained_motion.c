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
#include <stdlib.h>
#include "tinkervision.h"

TFV_Id motion = 1;
TFV_Id snap = 2;
TFV_Id result;

void tfcv_callback(TFV_Id id, TFV_Size x, TFV_Size y, TFV_Size width,
                   TFV_Size height, TFV_Context context) {
    printf("Motion at %d, %d\n", x, y);
}

int main(int argc, char* argv[]) {

    result = motiondetect_start(motion, tfcv_callback, NULL);
    printf("Id %d started: %s (%d)\n", motion, result_string(result), result);

    /* give api some time to actually start the modules */
    sleep(4);
    result = snapshot(snap);
    printf("Requested a snapshot with result %d: %s\n", result,
           result_string(result));
    /* chain(motion, snap); */
    result = snapshot(snap);
    printf("Requested a snapshot with result %d: %s\n", result,
           result_string(result));
    sleep(4);

    quit();
    return 0;
}
