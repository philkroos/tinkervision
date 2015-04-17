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

#include "tinkervision.h"

int main(int argc, char* argv[]) {
    TFV_Size width = 640;
    TFV_Size height = 480;

    TFV_Id result = preselect_framesize(width, height);

    printf ("PreselectFramesize: %d (%s)\n", result, result_string(result));
    sleep(1);

    result = camera_available();
    printf ("CameraAvailable: %d (%s)\n", result, result_string(result));
    sleep(1);

    result = start_idle();
    printf ("StartIdle: %d (%s)\n", result, result_string(result));
    sleep(2);

    result = get_resolution(&width, &height);
    printf ("GetResolution: %d (%s)\n", result, result_string(result));
    printf ("WxH: %lux%lu\n", (long unsigned)width, (long unsigned)height);
    sleep(1);

    result = quit();
    printf ("Quit: %d (%s)\n", result, result_string(result));
    sleep(1);

    return 0;
}
