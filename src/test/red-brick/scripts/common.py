from time import sleep

def run(red):
    ok = red.vision_camera_available()

    if ok != 0:
        raise Exception("vision::CameraAvailable returned error: " + str(ok))

    ok = red.vision_camera_id_available(0)
    print "Camera 0 available: ", ok
    ok = red.vision_camera_id_available(1)
    print "Camera 1 available: ", ok

    ok = red.vision_camera_id_select(1)
    print "Camera 1 selected: ", ok

    sleep(1)
    ok = red.vision_set_framesize(640, 480)

    if ok != 0:
        raise Exception("vision::SetFramesize returned error: " + str(ok))

    sleep(1)
    ok = red.vision_start_idle()

    if ok != 0:
        raise Exception("vision::StartIdle returned error: " + str(ok))

    sleep(1)
    width = 0
    height = 0
    ok, width, height = red.vision_get_framesize()

    if ok != 0:
        raise Exception("vision::GetFramesize returned error: " + str(ok))

    print "Resolution is " + str(width) + "x" + str(height)

    sleep(1)
