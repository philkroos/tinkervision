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

void tfcv_callback(TFV_Id id, TFV_Size x, TFV_Size y, TFV_Context context) {
    printf("Executing id %d\n", id);
}

int main(int argc, char* argv[]) {
    TFV_Id ids_count = 10;
    TFV_Id result;

    /* don't matter */
    TFV_Byte min_hue = 0;
    TFV_Byte max_hue = 10;

    int i, first, second;

    for (i = 0; i < ids_count; i++) {
        result = colormatch_start(i, min_hue, max_hue, tfcv_callback, NULL);
        printf("Id %d started: %s (%d)\n", i, result_string(result), result);
    }

    /* give api time to actually start the modules */
    sleep(1);

    srand(time(NULL));
    for (i = 0; i < ids_count * 2; i++) {
        first = rand() % ids_count;
        second = rand() % ids_count;

        result = chain(first, second);
        printf("Chaining %d -> %d...: %s (%d)\n", first, second,
               result_string(result), result);

        sleep(1);
    }

    quit();
    return 0;
}
