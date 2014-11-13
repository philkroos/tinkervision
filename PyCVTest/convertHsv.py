import numpy as np
import cv2

b = g = r = 0
bgr = np.uint8([[[b, g, r]]])
hsv = cv2.cvtColor(bgr, cv2.COLOR_BGR2HSV)

print bgr, "to", hsv

b = g = r = 125
bgr = np.uint8([[[b, g, r]]])
hsv = cv2.cvtColor(bgr, cv2.COLOR_BGR2HSV)

print bgr, "to", hsv

b = g = r = 255
bgr = np.uint8([[[b, g, r]]])
hsv = cv2.cvtColor(bgr, cv2.COLOR_BGR2HSV)

print bgr, "to", hsv

h = 0
s = v = 255
hsv = np.uint8([[[h, s, v]]])
bgr = cv2.cvtColor(hsv, cv2.COLOR_BGR2HSV)

print hsv, "to", bgr

h = 255
s = v = 255
hsv = np.uint8([[[h, s, v]]])
bgr = cv2.cvtColor(hsv, cv2.COLOR_BGR2HSV)

print hsv, "to", bgr
