from time import sleep

def visioncallback(id, x, y, width, height, string):
    print "visioncallback", id, x, y, width, height, string

def run(red):
    print "Starting color- and motionmatch modules"

    id, ok = red.vision_module_start("colormatch")

    if ok != 0:
        raise Exception("VisionModuleStart returned error: " + str(ok))

    print "Colormatch started with id", id

    ok = red.vision_parameter_set(id, "min-hue", 20)

    if ok != 0:
        raise Exception("VisionParameterSet returned error: " + str(ok))

    ok = red.vision_parameter_set(id, "max-hue", 40)

    if ok != 0:
        raise Exception("VisionParameterSet returned error: " + str(ok))

    ok, max_hue = red.vision_parameter_get(id, "max-hue")

    if ok != 0:
        raise Exception("VisionParameterGet returned error: " + str(ok))

    ok, min_hue = red.vision_parameter_get(id, "min-hue")

    if ok != 0:
        raise Exception("VisionParameterGet returned error: " + str(ok))

    print "Got min max:", min_hue, max_hue

    red.register_callback(red.CALLBACK_VISION_MODULE, visioncallback)

    sleep(5)
    exit(0)

    id2, ok = red.vision_module_start("motion")

    if ok != 0:
        raise Exception("vision::MotionStart returned error: " + str(ok))

    print "Motion started with id", id2
    sleep(30)

    print "Stopping both modules"
