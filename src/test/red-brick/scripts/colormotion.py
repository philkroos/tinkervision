from time import sleep

def colorcallback(id, x, y):
    print "colorcallback", x, y

def motioncallback(id, x, y, width, height):
    print "motioncallback", x, y, width, height

def run(red):
    print "Starting color- and motionmatch modules"

    ok = red.vision_module_start(1, "colormatch")

    if ok != 0:
        raise Exception("VisionModuleStart returned error: " + str(ok))

    ok = red.vision_parameter_set(1, "min-hue", 20)

    if ok != 0:
        raise Exception("VisionParameterSet returned error: " + str(ok))

    ok = red.vision_parameter_set(1, "max-hue", 40)

    if ok != 0:
        raise Exception("VisionParameterSet returned error: " + str(ok))

    ok, max_hue = red.vision_parameter_get(1, "max-hue")

    if ok != 0:
        raise Exception("VisionParameterGet returned error: " + str(ok))

    ok, min_hue = red.vision_parameter_get(1, "min-hue")

    if ok != 0:
        raise Exception("VisionParameterGet returned error: " + str(ok))

    print "Got min max:", min_hue, max_hue

    red.register_callback(red.CALLBACK_VISION_POINT, colorcallback)

    sleep(5)

    ok = red.vision_quit()

    if ok != 0:
        raise Exception("VisionQuit returned error: " + str(ok))

    exit(0)

    ok = red.vision_module_start(1, "motion")

    if ok != 0:
        raise Exception("vision::MotionStart returned error: " + str(ok))

    red.register_callback(red.CALLBACK_VISION_MOTION, motioncallback)

    # Wait a while, then quit controlled.
    sleep(30)

    print "Stopping both modules"
