def run(red):
    available = red.vision_camera_available()

    if available == 0: # vision-library returns 0 as value for OK
        print "TinkerVision is available"
    else:
        print "Tinkervision returned error: " + str(available)
