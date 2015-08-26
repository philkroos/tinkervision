#!/usr/bin/python

import sys
import subprocess
import pick_color

if __name__ == '__main__':
    cam_id = int(sys.argv[1]) if (len(sys.argv) >= 2) else 0
    h,s,v = pick_color.run(cam_id, 4)
    subprocess.call("./tfv-colormatch " + str(h) + " " + str(s) + " " + str(v) + " 8", shell=True)
