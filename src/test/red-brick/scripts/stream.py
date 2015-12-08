from time import sleep

def run(red):
    # Check if the cam is online. Not necessary usually.
    ok = red.vision_camera_available()

    if ok != 0:
        raise Exception("vision::CameraAvailable returned error: " + str(ok))

    sleep(1)

    # Set framesize to a streamable size
    ok = red.vision_set_framesize(320, 240)

    if ok != 0:
        raise Exception("vision::SetFramesize returned error: " + str(ok))

    # Stream for 60 seconds
    ok, id = red.vision_module_start("stream")

    if ok != 0:
        raise Exception("vision::Stream returned error: " + str(ok))

    sleep(1)
    ok = 1
    while ok > 0:
        ok, url = red.vision_string_parameter_get(id, "url")

    print "Streaming for 60 seconds on", url
    sleep(120)
    red.vision_remove_all_modules()
