import cv2
import numpy as np

from time import sleep

hue = 0

class HuePicker:
    def __init__(self, wname):
        self.hue = 0
        self.h = self.s = self.v = 0
        self.x = self.y = 0
        cv2.setMouseCallback(wname, self.update_hue)

    def update_hue(self, event, x, y, flag, param):
        if self.hsv == None:
            return
        if (event == cv2.EVENT_MOUSEMOVE):
            self.h = self.hsv[y, x][0]
            self.s = self.hsv[y, x][1]
            self.v = self.hsv[y, x][2]
            self.x = x
            self.y = y

    def get_hue(self):
        return self.hue

cap = cv2.VideoCapture(1)
wname = 'frame'
cv2.namedWindow(wname)
hp = HuePicker(wname)
face = cv2.FONT_HERSHEY_SIMPLEX
scale = .5
thickness =2
color = 255
size, base = cv2.getTextSize("HSV at %d/%d: 255-255-255" % (111, 111), face, scale, thickness)
width = size[0]
height = size[1]

# hsv range (pink)
range0_low = np.array([170, 50, 50])
range0_high = np.array([180, 255, 255])
range1_low = np.array([0, 50, 50])
range1_high = np.array([10, 255, 255])

# hsv range (light blue)
range0_low = np.array([97, 50, 50])
range0_high = np.array([105, 255, 255])
range1_low = None #np.array([95, 50, 50])
#range1_high = np.array([100, 255, 255])

def contour_based(mask):
    contours, hierarchy = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    rects = []
    for contour in contours:
        poly = cv2.approxPolyDP(contour, 3, True)
        rects.append(cv2.boundingRect(poly))

    return rects


def flood_based(frame, hue):
    h, w = hp.hsv.shape[:2]
    mask = np.zeros((h+2, w+2), np.uint8)
    mask[:] = 0
    rects = []
    for x in range(w):
        for y in range(h):
            if hue_similar(frame[y][x], hue):
                print str(frame[y][x]) + " is similar to", hue, "at", x, ",", y
                ret, rect = cv2.floodFill(frame, mask, (y, x), (255, 255, 255),
                                          3, 3, cv2.FLOODFILL_FIXED_RANGE)
                print "Floodfilled", rect[2], ",", rect[3]
                cv2.imshow(wname, frame)
                sleep(1)
                
                rects.append(rect)

    return rects

def hue_similar(pt1, value):
    print str(pt1[0]), "is similar to", value
    return abs(pt1[0] - value) < 7

video = True
counter = 0
while True:

    if video:
        _, frame = cap.read()
    else:
        frame = cv2.imread("./res10.jpg")

    hp.hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    rows, cols, _ = frame.shape
    texty = rows - 10 - height
    textx = 10
    cv2.putText(frame, "HSV at %d/%d: %d-%d-%d" % (hp.x, hp.y, hp.h, hp.s, hp.v),
                (textx, texty), face, scale, color, thickness)

    if range1_low:
        mask0 = cv2.inRange(hp.hsv, range0_low, range0_high)
        mask1 = cv2.inRange(hp.hsv, range1_low, range1_high)
        mask = cv2.bitwise_or(mask0, mask1)
    else:
        mask = cv2.inRange(hp.hsv, range0_low, range0_high)

    kernel = np.ones((5, 5), np.uint8)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    res = cv2.bitwise_and(frame, frame, mask = mask)
    res_hsv = cv2.cvtColor(res, cv2.COLOR_BGR2HSV)

    rects = contour_based(mask)
    # not good. don't know where to start flooding.
    #rects = flood_based(res_hsv, (range0_low[0] + range0_high[0]) / 2)

    if rects:
        rect = max(rects, key = lambda r: r[2] * r[3])
        x, y, w, h = rect
        cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 255, 255), 4)
    cv2.imshow(wname, frame)
    cv2.imshow('mask', mask)
    cv2.imshow('res', res)
    counter = counter + 1
    #cv2.imwrite("./res" + str(counter) + ".jpg", res)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()

