def run(red):
    available = red.vision_camera_available()

    if available == 0: # vision-library returns 0 as value for OK
        print "TinkerVision is available"
    else:
        print "Tinkervision returned error: " + str(available)
        exit(-1)

    ok, user_path = red.vision_lib_user_load_path();

    if ok != 0:
        print "Could not get user load path"
        exit(-1)

    [ok, system_path] = red.vision_lib_system_load_path();

    if ok != 0:
        print "Could not get system load path"
        exit(-1)

    print "System module path:", system_path
    print "User module path:", user_path

    [ok, count] = red.vision_libs_count();

    if ok != 0:
        print "Could not get number of available modules"
        exit(-1)

    print "Total available modules:", count
