from time import sleep

# UID of the Redbrick
uid = "2SYC7p"
# IP of the redbrick (set to localhost if this script shall run on the redbrick)
host = "192.168.178.21"
# Port the redbrick is listening on (default)
port = 4223

# Libraries to communice with the brick
from tinkerforge.ip_connection import IPConnection
from tinkerforge.brick_red import BrickRED

try:
    # Establish communication channel
    ipcon = IPConnection()
    ipcon.connect(host, port)
except Exception, e:
    print "Failed to connect to " + host + ":" + str(port)
    exit(-1)


try:
    # Instantiate the object representation of the RedBrick
    red = BrickRED(uid, ipcon)

    ok = red.vision_camera_available()

    if ok != 0:
        raise Exception("vision::CameraAvailable returned error: " + str(ok))

    sleep(1)
    ok = red.vision_preselect_framesize(640, 480)

    if ok != 0:
        raise Exception("vision::PreselectFramesize returned error: " + str(ok))

    sleep(1)
    ok = red.vision_start_idle()

    if ok != 0:
        raise Exception("vision::StartIdle returned error: " + str(ok))

    sleep(1)
    width = 0
    height = 0
    ok, width, height = red.vision_get_resolution()

    if ok != 0:
        raise Exception("vision::GetResolution returned error: " + str(ok))

    print "Resolution is " + str(width) + "x" + str(height)

    sleep(1)


except Exception, e:
    print "Exception occured: " + str(e)

finally:
    ok = red.vision_quit()

    if ok != 0:
        print "vision::Quit returned error: " + str(ok)

    ipcon.disconnect()
