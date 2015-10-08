from time import sleep

def run(red):
    # Check if the cam is online. Not necessary usually.
    ok = red.vision_camera_available()

    if ok != 0:
        raise Exception("vision::CameraAvailable returned error: " + str(ok))

    sleep(1)

    # Set framesize to a streamable size
    ok = red.vision_preselect_framesize(640, 480)

    if ok != 0:
        raise Exception("vision::PreselectFramesize returned error: " + str(ok))

    # Stream for 60 seconds
    id = 0
    ok = red.vision_stream(id)

    if ok != 0:
        raise Exception("vision::Stream returned error: " + str(ok))

    print "Streaming for 60 seconds"
    sleep(60)
