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

#include "tinkervision/tinkervision.h"

int main(int argc, char* argv[]) {
    int8_t id;
    char url[TV_STRING_SIZE];
    int16_t result = tv_module_start("stream", &id);
    printf("Started streamer with result %d: %s\n", result,
           tv_result_string(result));

    if (!result) {
        sleep(1);
        result = tv_module_get_string_parameter(id, "url", url);
        printf("Streaming on %s (%s)\n", url, tv_result_string(result));
        sleep(50);
    }

    return 0;
}
