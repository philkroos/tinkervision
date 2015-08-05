from time import sleep

def run(red):
    ok = red.vision_camera_available()
    if ok != 0:
        raise Exception("vision::CameraAvailable returned error: " + str(ok))

    ok = red.vision_quit()

    if ok != 0:
        raise Exception("vision::Quit returned error: " + str(ok))
