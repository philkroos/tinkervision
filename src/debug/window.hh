/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014-2015 philipp.kroos@fh-bielefeld.de

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

#if defined DEBUG

#include <iostream>
#include <map>

#include <opencv2/opencv.hpp>

#include "tinkervision_defines.h"

namespace tfv {
class Window {
private:
    std::map<TFV_Id, std::string> windows_;
    std::string prefix = "Camera ";

public:
    void update(TFV_Id id, TFV_ImageData* data, int rows, int columns,
                int format = CV_8UC3) {
        if (windows_.find(id) == windows_.end()) {
            windows_[id] = prefix + std::to_string(id) + " ";
            cv::namedWindow(windows_[id]);
        }
        cv::Mat frame(rows, columns, format, data);
        cv::imshow(windows_[id], frame);
        cv::waitKey(100);  // skip time to give window-update thread a chance
    }

    void update(TFV_Id id, cv::Mat const& image) {
        if (windows_.find(id) == windows_.end()) {
            windows_[id] = prefix + std::to_string(id) + " ";
            cv::namedWindow(windows_[id]);
        }
        cv::imshow(windows_[id], image);
        cv::waitKey(100);  // skip time to give window-update thread a chance
    }
};
}

#endif  // DEV

#endif /* WINDOW_H */
