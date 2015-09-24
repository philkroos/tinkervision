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

#include <iostream>
#include <map>
#include <opencv2/opencv.hpp>

#include "tinkervision/tinkervision_defines.h"

namespace tfv {
class Window {
private:
    using Image = struct Image {
        cv::Mat* frame = nullptr;
        std::string name;
    };

    std::map<TFV_Id, Image> windows_;
    std::string prefix = "Camera ";

public:
    ~Window(void) {
        for (auto& win : windows_) {
            if (win.second.frame) {
                delete win.second.frame;
            }
        }
    }

    void update(TFV_Id id, TFV_ImageData* data, char const* text = "") {
        auto win = windows_.find(id);

        if (win == windows_.end() or not win->second.frame) {
            return;
        }

        win->second.frame->data = data;
        if (std::string(text) != std::string("")) {
            put_text(id, text);
        }
        cv::imshow(win->second.name, *(win->second.frame));
        cv::waitKey(100);  // skip time to give window-update thread a chance
    }

    void update(TFV_Id id, TFV_ImageData* data, int rows, int columns,
                char const* text = "", int format = CV_8UC3) {

        auto win = windows_.find(id);

        if (win == windows_.end()) {
            windows_[id] = {};
            windows_[id].frame = nullptr;
            windows_[id].name = prefix + std::to_string(id) + " ";
            cv::namedWindow(windows_[id].name);
            win = windows_.find(id);
        }

        if (not win->second.frame) {
            win->second.frame = new cv::Mat(rows, columns, format, data);
        } else if ((win->second.frame->rows != rows) or
                   (win->second.frame->cols != columns) or
                   (win->second.frame->type() != format)) {
            return;
        } else {
            win->second.frame->data = data;
        }

        if (std::string(text) != std::string("")) {
            put_text(id, text);
        }
        cv::imshow(win->second.name, *(win->second.frame));
        cv::waitKey(100);  // skip time to give window-update thread a chance
    }

    void update(TFV_Id id, cv::Mat const& image, int rows, int columns) {
        auto win = windows_.find(id);
        if (windows_.find(id) == windows_.end()) {
            windows_[id].name = prefix + std::to_string(id) + " ";
            cv::namedWindow(windows_[id].name);
        }
        win = windows_.find(id);

        cv::imshow(win->second.name, *(win->second.frame));
        cv::waitKey(100);  // skip time to give window-update thread a chance
    }

    void put_text(TFV_Id id, const char* text) {
        auto win = windows_.find(id);
        if (win == windows_.end() or not win->second.frame) {
            return;
        }

        cv::Point textpos(20, win->second.frame->rows - 20);
        cv::Scalar textcolor(200, 200, 200);
        int thickness = 1;

        cv::putText(*(win->second.frame), text, textpos,
                    cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, textcolor, thickness,
                    CV_AA);
        cv::imshow(win->second.name, *(win->second.frame));
    }

    void wait_for_input(void) { cv::waitKey(0); }
};
}

#endif /* WINDOW_H */
