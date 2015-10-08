#!/usr/bin/python
import sys

# RedBrick resource wrapper
from rb_setup import RedBrick
# available testfiles
import shutdown, availability, stream, common, colormotion, scenes

testfiles = {
    "shutdown": shutdown,
    "stream": stream,
    "availability": availability,
    "common": common,
    "colormotion": colormotion,
    "scenes": scenes
    }


def usage_exit(prog):
    print "Usage 1 (on RedBrick):", prog, "rb-uid testfilename", \
          "[host = localhost [port = 4223]]"
    print "Usage 0 (local):", prog, "- testfilename"

if __name__ == '__main__':
    host = "localhost"
    port = 4223
    prog = None
    if len(sys.argv) < 3:
        usage_exit(sys.argv[0])

    filename = sys.argv[2]
    if sys.argv[1] != '-':
        uid = sys.argv[1]

        try:
            prog = testfiles[filename]
            host =  sys.argv[3] if len(sys.argv) > 3 else host
            port = int(sys.argv[4]) if len(sys.argv) > 4 else port

        except KeyError, e:
            print "No such file", filename + ".py or not registered"
            exit(-1)

        with RedBrick(uid, host, port) as rb:
            prog.run(rb)

    else:
        try:
            prog = testfiles[filename]
        except KeyError, e:
            print "No such file", filename + ".py or not registered"
            exit(-1)

        with Lib() as lib:
            prog.run(lib)
