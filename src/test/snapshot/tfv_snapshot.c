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
    TFV_Id id = 0;
    TFV_Size width, height;
    TFV_ModuleResult m_result;

    TFV_Result result = module_start("snapshot", id);
    printf("Load module snapshot: result %d: %s\n", result,
           result_string(result));

    result = get_resolution(&width, &height);
    printf("Framesize is %dx%d\n", width, height);

    /* Wait for a moment to have the Snapshot module executed at least once */
    sleep(1);

    result = get_result(id, &m_result);
    printf("Snapshot result: %d (%s)\n", result, result_string(result));
    if (result == TFV_OK) {
        printf("Snapshot is %s\n", m_result.string);
    }

    /*
     Check image on http://rawpixels.net/ Settings:
     - Predefined format: YUV420p
     - Pixel format: YVU
     - Deselect 'Alpha first'
     - width/height: see output
     - Pixel Plane: Planar
    */

    return 0;
}
