from time import sleep

def callback(id, x, y):
    print "Callback for id:", id


def start_module(red, id):
    ok = red.vision_colormatch_start(id, 50, 100)
    if ok != 0:
        raise Exception("vision::ColormatchStart returned error: " + str(ok))
    print "Started module", id
    sleep(1)
    red.register_callback(red.CALLBACK_VISION_COLORMATCH, callback)

def start_scene(red, module):
    scene, ok = red.vision_scene_start(1)
    if ok != 0:
        raise Exception("vision::SceneStart returned error: " + str(ok))
    print "Started scene", scene, "from module", module
    sleep(1)
    return scene

def scene_add(red, scene, module):
    ok = red.vision_scene_add(scene, module)
    if ok != 0:
        raise Exception("vision::SceneAdd returned error: " + str(ok))
    print "Added", module, "to scene", scene
    sleep(1)

def run(red):
    start_module(red, 1)
    start_module(red, 2)
    start_module(red, 3)
    start_module(red, 4)
    start_module(red, 5)
    start_module(red, 6)
    start_module(red, 7)
    start_module(red, 8)

    scene1 = start_scene(red, 1)
    scene2 = start_scene(red, 1)
    scene3 = start_scene(red, 1)

    scene_add(red, scene1, 2)
    scene_add(red, scene2, 2)
    scene_add(red, scene1, 3)
    scene_add(red, scene2, 4)
    scene_add(red, scene3, 5)

    sleep(5)
    red.vision_quit()
