def run(red):
    ok  = red.vision_remove_all_modules();

    if ok == 1: # removal in progress
        print "Tinkervision is busy"

    if ok != 0: # vision-library returns 0 as value for OK
        print "Tinkervision returned error: " + str(ok)
