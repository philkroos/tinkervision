/// \file window.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Window declaration usable in debugging contexts.
///
/// This file is part of Tinkervision - Vision Library for Tinkerforge Redbrick
/// \sa https://github.com/Tinkerforge/red-brick
///
/// \copyright
///
/// This program is free software; you can redistribute it and/or
/// modify it under the terms of the GNU General Public License
/// as published by the Free Software Foundation; either version 2
/// of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
/// USA.

#ifndef WINDOW_H
#define WINDOW_H

#if defined DEBUG_WINDOW

#include <iostream>
#include <map>

#include <opencv2/opencv.hpp>

#include "tinkervision_defines.h"

namespace tv {
class Window {
private:
    std::map<int8_t, std::string> windows_;
    std::string prefix = "Camera ";

public:
    void update(int8_t id, uint8_t const* data, int rows, int columns,
                int format = CV_8UC3) {
        assert(rows > 0 and columns > 0 and data);

        if (windows_.find(id) == windows_.end()) {
            windows_[id] = prefix + std::to_string(id) + " ";
            cv::namedWindow(windows_[id]);
        }
        cv::Mat frame(rows, columns, format, (void*)data);
        cv::imshow(windows_[id], frame);
        cv::waitKey(100);  // skip time to give window-update thread a chance
    }

    void update(int8_t id, cv::Mat const& image) {
        assert(image.rows > 0 and image.cols > 0 and image.data);

        if (windows_.find(id) == windows_.end()) {
            windows_[id] = prefix + std::to_string(id) + " ";
            cv::namedWindow(windows_[id]);
        }
        cv::imshow(windows_[id], image);
        cv::waitKey(100);  // skip time to give window-update thread a chance
    }
};
}

#endif  // DEBUG_WINDOW

#endif /* WINDOW_H */
