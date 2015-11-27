from time import sleep

def check(code, msg):
    if code != 0:
        print "Error in context:", msg, ":", code
        exit(-1)

def run(red):
    ok = red.vision_camera_available()
    check(ok, "Available")

    ok = red.vision_remove_all_modules()
    check(ok, "RemoveAll")

    ok, user_path = red.vision_lib_get_user_load_path();
    check(ok, "UserPath")

    [ok, system_path] = red.vision_lib_get_system_load_path();
    check(ok, "SystemLoadPath")

    print "System module path:", system_path
    print "User module path:", user_path

    [ok, count] = red.vision_libs_count();
    check(ok, "LibsCount")

    print "Total available modules:", count

    for i in range(count):
        [ok, name, path] = red.vision_lib_name_path(i)
        check(ok, "LibNamePath")

        print "Describing module", path + name

        [ok, p_count] = red.vision_lib_parameters_count(name)
        check(ok, "ParameterCount")

        print "--", p_count, "parameters"

        for p in range(p_count):
            [ok, p_name, p_type, p_min, p_max, p_def] = \
              red.vision_lib_parameter_describe(name, p)
            check(ok, "ParameterDescribe")

            print "-- Parameter", str(p) + ":", p_name
            print "-- type:", ("string" if p_type == 1 else "number")
            if p_type != 1:
                print "-- min/default/max:", str(p_min) + "/" + str(p_def) + "/" + str(p_max)

    ok = red.vision_start_idle()
    check(ok, "StartIdle")

    print "Api running in idle mode"

    [ok, width, height] = red.vision_get_framesize()
    check(ok, "GetFramesize")

    print "Working with framesize", str(width) + "x" + str(height)

    ok = red.vision_set_framesize(320, 240)
    check(ok, "SetFramesize")

    [ok, width, height] = red.vision_get_framesize()
    check(ok, "GetFramesize")

    print "Set framesize to", str(width) + "x" + str(height)
    print "Running for 2 seconds"
    sleep(2)

    [ok, period] = red.vision_get_frameperiod()
    check(ok, "GetFrameperiod")

    if period > 0:
        print "Current framerate:", str(1.0/period)

    inv_rate = 1000
    print "Setting framerate to", str(1.0/inv_rate)
    ok = red.vision_request_frameperiod(inv_rate)
    check(ok, "RequestFrameperiod")

    print "Running for 2 seconds"
    sleep(2)

    [ok, period] = red.vision_get_frameperiod()
    check(ok, "GetFrameperiod")

    if period > 0:
        print "Current framerate:", str(1.0/period)

    ok = red.vision_remove_all_modules()
    check(ok, "RemoveAll")

    print "Removed all modules, done"
