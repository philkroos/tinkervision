/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014 philipp.kroos@fh-bielefeld.de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef WINDOW_H
#define WINDOW_H

#ifdef DEV

#include <iostream>

#include <opencv2/opencv.hpp>

#include "tinkervision_defines.h"

namespace tfv {
class Window {
private:
    std::string name;

public:
    explicit Window(std::string name) : name(name) { cv::namedWindow(name); }

    void update(TFV_ImageData* data, int rows, int columns) {
        cv::Mat frame(rows, columns, CV_8UC3, data);
        cv::imshow(name, frame);
        cv::waitKey(100);  // skip time to give window-update thread a chance
    }
};
}

#endif  // DEV

#endif /* WINDOW_H */
