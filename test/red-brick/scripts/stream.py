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
        raise "vision::CameraAvailable returned error: " + str(ok)

    ok = red.vision_preselect_framesize(640, 480)

    if ok != 0:
        raise "vision::PreselectFramesize returned error: " + str(ok)

    id = 0
    ok = red.vision_stream(id)

    if ok != 0:
        raise "vision::Stream returned error: " + str(ok)

    print "Streaming for 30 seconds"
    sleep(30)

    ok = red.vision_pause_id(id)

    if ok != 0:
        raise "vision::StopId returned error: " + str(ok)

    # Actually no need to do this explicitly
    ok = red.vision_quit()

    if ok != 0:
        raise "vision::Quit returned error: " + str(ok)

except Exception, e:
    print "Exception occured: " + str(e)

finally:
    ipcon.disconnect()
