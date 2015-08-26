from time import sleep

def colorcallback(id, x, y):
    print "colorcallback", x, y

def motioncallback(id, x, y, width, height):
    print "motioncallback", x, y, width, height

def run(red):
    print "Starting color- and motionmatch modules"

    ok = red.vision_colormatch_start(1, 20, 27)

    if ok != 0:
        raise Exception("vision::ColormatchStart returned error: " + str(ok))

    sleep(2)
    ok, hmin, hmax = red.vision_colormatch_get(1)

    if ok != 0:
        raise Exception("vision::ColormatchGet returned error: " + str(ok))

    print "Got min max:", hmin, hmax

    ok = red.vision_motion_start(2)

    if ok != 0:
        raise Exception("vision::MotionStart returned error: " + str(ok))

    red.register_callback(red.CALLBACK_VISION_COLORMATCH, colorcallback)
    red.register_callback(red.CALLBACK_VISION_MOTION, motioncallback)

    # Wait a while, then quit controlled.
    sleep(30)

    print "Stopping both modules"
    ok = red.vision_quit()

    if ok != 0:
        raise Exception("vision::Quit returned error: " + str(ok))
