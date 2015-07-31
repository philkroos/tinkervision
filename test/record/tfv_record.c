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

/* This program has to enable and disable the camera 4 times, each time taking a
 * single snapshot (in program folder, increasing timestamps).
 */
int main(int argc, char* argv[]) {
    TFV_Id id = 20;

    /* Take a snapshot with a one-shot-then-release-module.
       As of 04-04-2015, the output should show the destructor being called */
    TFV_Result result = singleshot();
    printf("Requested a snapshot with result %ld: %s\n", result,
           result_string(result));

    sleep(2);

    /* Another one, same as above. */
    result = singleshot();
    printf("Requested a snapshot with result %ld: %s\n", result,
           result_string(result));

    sleep(2);

    /* Now assign an id, the module should stay alive but disabled (i.e. cam
     * down) */
    result = snapshot(id);
    printf("Requested a snapshot with result %ld: %s\n", result,
           result_string(result));

    sleep(2);
    result = snapshot(id);
    printf("Requested a snapshot with result %ld: %s\n", result,
           result_string(result));

    sleep(1);
    return 0;
}
