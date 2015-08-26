import cv2
import numpy as np
import sys
from time import sleep

class Hue:
    h = 0
    s = 0
    v = 0

class HueAverage:
    def __init__(self, wname, wwidth, wheight, avgSize):
        self.avgSize = avgSize
        self.x = self.y = 0
        self.at = Hue();
        self.avg = Hue();
        self.clicked = False
        self.width = wwidth
        self.height = wheight

    def mouseHandler(self, event, x, y, flag, param):
        if self.hsv == None:
            return
        if (event == cv2.EVENT_LBUTTONDOWN):
            self.x = x
            self.y = y
            xmin = int(max(0, x - self.avgSize))
            xmax = int(min(self.width, x + self.avgSize))
            ymin = int(max(0, y - self.avgSize))
            ymax = int(min(self.height, y + self.avgSize))
            self.at.h = self.hsv[y, x][0]
            self.at.s = self.hsv[y, x][1]
            self.at.v = self.hsv[y, x][2]
            h = s = v = 0
            for x in range(xmin, xmax):
                for y in range(ymin, ymax):
                    h += self.hsv[y, x][0]
                    s += self.hsv[y, x][1]
                    v += self.hsv[y, x][2]
            values = len(range(xmin, xmax)) * len(range(ymin, ymax))
            self.avg.h = h / values
            self.avg.s = s / values
            self.avg.v = v / values


wname = 'frame'
face = cv2.FONT_HERSHEY_SIMPLEX
scale = .5
thickness =2
color = 255
size, base = cv2.getTextSize("HSV at %d/%d: 255-255-255. Avg around: 255-255-255" %
                             (111, 111), face, scale, thickness)
width = size[0]
height = size[1]


def run(cam_id, distance):
    print "Using cam", cam_id
    cap = cv2.VideoCapture(cam_id)
    cv2.namedWindow(wname)
    framewidth = cap.get(cv2.cv.CV_CAP_PROP_FRAME_WIDTH)
    frameheight = cap.get(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT)
    hp = HueAverage(wname, framewidth, frameheight, distance)
    cv2.setMouseCallback(wname, hp.mouseHandler)

    while True:
        ret, frame = cap.read()

        if ret != True:
            continue
        hp.hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        rows, cols, _ = frame.shape
        texty = rows - 10 - height
        textx = 10
        cv2.putText(frame, "HSV at %d/%d: %d-%d-%d. Avg around: %d-%d-%d" %
                    (hp.x, hp.y, hp.at.h, hp.at.s, hp.at.v,
                     hp.avg.h, hp.avg.s, hp.avg.v),
                    (textx, texty), face, scale, color, thickness)

        cv2.imshow(wname, frame)
        if cv2.waitKey(1) & 0xFF == ord('q') or hp.clicked:
            break

    cap.release()

    return [hp.avg.h, hp.avg.s, hp.avg.v]
