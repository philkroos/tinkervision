import sys
import subprocess
import pick_color

if __name__ == '__main__':
    cam_id = int(sys.argv[1]) if len(sys.argv) >= 2 else 0

    subprocess.call("./tfv-colormatch " + str(pick_color.run(cam_id)), shell=True)
