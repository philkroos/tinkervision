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
    /* printf("Executing id %d\n", id); */
}

TFV_Result result;

void scene_start(TFV_Id module, TFV_Scene* scene) {
    result = scene_from_module(module, scene);
    printf("Scene from %d as %ld: %s (%ld)\n",
           module, *scene, result_string(result), result);
    sleep(1);
}

void scene_add(TFV_Scene scene, TFV_Id module) {
    result = scene_add_module(scene, module);
    printf("Add %d to scene %ld: %s (%ld)\n",
           module, scene, result_string(result), result);
    sleep(1);
}

int main(int argc, char* argv[]) {

    int i;

    TFV_Scene scene_1, scene_2;/*, scene_3, scene_4,
                                 scene_5, scene_6, scene_7, scene_8;*/

    size_t ids_count = 10;

    /* don't matter */
    TFV_Byte min_hue = 0;
    TFV_Byte max_hue = 10;

    printf ("--> This currently FAILS due to wrong detection of a scenes" \
            " last node\n--> See scenetrees.cc\n");

    /* start ids_count colormatch modules to be used in scenes. */
    for (i = 0; i < ids_count; i++) {
        result = colormatch_start(i, min_hue, max_hue, tfcv_callback, NULL);
        printf("Id %d started: %s (%ld)\n", i, result_string(result), result);
    }

    /* give api time to actually start the modules */
    sleep(1);

    /* tree 1 */
    scene_start(0, &scene_1);
    scene_start(0, &scene_2);
    /*
    scene_start(0, &scene_3);
    scene_start(0, &scene_4);
    scene_start(0, &scene_5);
    scene_start(0, &scene_6);
    */
    /* tree 2
    scene_start(1, &scene_7);
    scene_start(1, &scene_8);
 */
    /* tree 1, scene 1 */
    scene_add(scene_1, 1);
    scene_add(scene_1, 4);
    /* tree 1, scene 2 */
    /*    scene_add(scene_2, 1); sharing 1 */
    scene_add(scene_2, 5); /* diverging from 1 */
    scene_add(scene_2, 6);
#ifdef UNDEFINED
    /* tree 1, scene 3 */
    scene_add(scene_3, 1); /* sharing 1 and 2 */
    scene_add(scene_3, 5); /* sharing 2 */
    scene_add(scene_3, 7); /* diverging from 2 */
    /* tree 1, scene 4 */
    scene_add(scene_4, 2); /* diverging from 1 */
    /* tree 1, scene 5 */
    scene_add(scene_5, 3); /* diverging from 1 */
    scene_add(scene_5, 8);
    /* tree 1, scene 6 */
    scene_add(scene_5, 3); /* sharing 5 */
    scene_add(scene_5, 9); /* diverging from 5 */
#endif

    sleep(3);

    quit();
    return 0;
}
