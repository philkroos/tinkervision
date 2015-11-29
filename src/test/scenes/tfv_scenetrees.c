/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2015 philipp.kroos@fh-bielefeld.de

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
#include "tinkervision/tinkervision.h"

void callback(int8_t id, TV_ModuleResult result, void* context) {
    printf("Executing id %d\n", id);
}

int16_t result;

void scene_start(int8_t module, int16_t* scene) {
    result = tv_scene_from_module(module, scene);
    printf("Scene from %d as %d: %s (%d)\n", module, *scene,
           tv_result_string(result), result);
    sleep(1);
}

void scene_add(int16_t scene, int8_t module) {
    result = tv_scene_add_module(scene, module);
    printf("Add %d to scene %d: %s (%d)\n", module, scene,
           tv_result_string(result), result);
    sleep(1);
}

void colormatch_start(int8_t id, int min_hue, int max_hue) {
    int16_t result = tv_module_start("colormatch", &id);
    printf("Colormatch Id %d Start: %d (%s)\n", id, result,
           tv_result_string(result));
    if (result != TV_OK) {
        return;
    }
    result = tv_module_set_numerical_parameter(id, "min-hue", min_hue);
    printf("Set min-hue: %d (%s)\n", result, tv_result_string(result));
    if (result != TV_OK) {
        return;
    }
    result = tv_module_set_numerical_parameter(id, "max-hue", max_hue);
    printf("Set max-hue: %d (%s)\n", result, tv_result_string(result));
    if (result != TV_OK) {
        return;
    }
    /* result = tv_set_callback(id, callback); */
    /* if (result != TV_OK) { */
    /*     printf("Setting the callback failed: %d (%s)\n", result, */
    /*            tv_result_string(result)); */
    /* } */
}

/*
  Creating the following two trees (nodes are module-ids):
              0            1
            / | \         / \
           1  2  3       2   3
          /|     |\
         4 5     8 9
          / \
         6   7
  corresponding to 8 scenes (count the leafs):
  0-1-4
  0-1-5-6
  0-1-5-7
  0-2
  0-3-8
  0-3-9
  1-2
  1-3
  which means that though 0 is being used in 6 scenes,
  it will only be executed once per image.

  The resulting trees are (currently) printed from
  SceneTrees::exec_all() to the logfile (/tmp/tv.log).
*/
int main(int argc, char* argv[]) {

    int i;
    int16_t result;

    int16_t scene_1, scene_2, scene_3, scene_4, scene_5, scene_6, scene_7,
        scene_8;

    size_t ids_count = 10;

    /* don't matter */
    uint8_t min_hue = 0;
    uint8_t max_hue = 10;

    /* start ids_count colormatch modules to be used in scenes. */
    for (i = 0; i < ids_count; i++) {
        colormatch_start(i, min_hue, max_hue);
    }

    result = tv_callback_enable_default(callback);
    printf("Set callback: Code %d (%s)\n", result, tv_result_string(result));

    /* give api time to actually start the modules */
    sleep(1);

    /* tree 1 */
    scene_start(0, &scene_1);
    scene_start(0, &scene_2);
    scene_start(0, &scene_3);
    scene_start(0, &scene_4);
    scene_start(0, &scene_5);
    scene_start(0, &scene_6);

    /* tree 2 */
    scene_start(1, &scene_7);
    scene_start(1, &scene_8);

    /* tree 1, scene 1 */
    scene_add(scene_1, 1);
    scene_add(scene_1, 4);
    /* tree 1, scene 2 */
    scene_add(scene_2, 1); /* sharing 1 */
    scene_add(scene_2, 5); /* diverging from 1 */
    scene_add(scene_2, 6);

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
    scene_add(scene_6, 3); /* sharing 5 */
    scene_add(scene_6, 9); /* diverging from 5 */

    /* tree 2 */
    scene_add(scene_7, 2);
    scene_add(scene_7, 3);

    sleep(3);

    tv_quit();
    return 0;
}
