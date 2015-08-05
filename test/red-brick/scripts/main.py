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


if __name__ == '__main__':
    host = "localhost"
    port = 4223
    prog = None
    if len(sys.argv) < 3:
        print "Usage:", sys.argv[0], "rb-uid testfilename", \
          "[host = localhost [port = 4223]]"
        exit(-1)

    uid = sys.argv[1]
    filename = sys.argv[2]
    try:
        prog = testfiles[filename]
        host =  sys.argv[3] if len(sys.argv) > 3 else host
        port = int(sys.argv[4]) if len(sys.argv) > 4 else port

    except KeyError, e:
        print "No such file", filename + ".py or not registered"
        exit(-1)

    with RedBrick(uid, host, port) as rb:
        prog.run(rb)
