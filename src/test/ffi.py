from ctypes import *

def error_quit(message):
    print message
    exit(-1)

def error_quit_maybe(result, message):
    if result != 0:
        print message
        exit(-1)

if __name__ == '__main__':
    try:
        ltv = CDLL("libtinkervision.so")

    except OSError:
        error_quit("Could not open the tinkervision library")


    result = ltv.start_idle() != 0
    error_quit_maybe(result, "StartIdle")

    width = c_int()
    height = c_int()
    result = ltv.get_resolution(byref(width), byref(height))
    error_quit_maybe(result, "GetResolution")

    print "Running with framesize " + str(width.value) + "X" + str(height.value)


    result = ltv.quit()
    error_quit_maybe(result, "Quit")
