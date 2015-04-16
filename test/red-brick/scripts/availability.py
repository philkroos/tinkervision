# UID of the Redbrick
uid = "2SYC7p"
# IP of the redbrick (set to localhost if this script shall run on the redbrick)
host = "192.168.0.48"
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
    print "Failed to connect to " + host + ":" + port
    exit(-1)


try:
    # Instantiate the object representation of the RedBrick
    red = BrickRED(uid, ipcon)

    available = red.vision_camera_available()

    if available == 0: # vision-library returns 0 as value for OK
        print "TinkerVision is available"
    else:
        print "Tinkervision returned error: " + available

    # Actually no need to do this explicitly
    red.vision_quit()

except Exception, e:
    print "Exception occured: " + str(e)

finally:
    ipcon.disconnect()
