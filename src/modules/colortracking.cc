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

#include "colortracking.hh"

#include <opencv2/opencv.hpp>

#ifdef DEV
#include <iostream>
#endif

// member functions

void tfv::Colortracking::execute(tfv::Image const& image) {
    const auto rows = image.height;
    const auto columns = image.width;
    const auto data = image.data;

    cv::Mat cv_image(rows, columns, CV_8UC3, data);
    cv::cvtColor(cv_image, cv_image, CV_BGR2HSV);
    cv::Mat mask(rows, columns, CV_8UC3);

    /*
    static int counter = 0;
    std::string imgname = "ctexec_" + std::to_string(counter++) + ".jpg";
    std::cout << "Writing cv_image " << imgname << std::endl;
    cv::imwrite(imgname, cv_image);
    */

    auto const split = min_hue > max_hue;
    cv::Scalar low(min_hue, min_saturation, min_value);
    cv::Scalar high(split ? max_hue0 : max_hue, max_saturation, max_value);

    if (split) {
        cv::Mat mask0(rows, columns, CV_8UC3);
        cv::Mat mask1(rows, columns, CV_8UC3);
        cv::inRange(cv_image, low, high, mask0);
        low[0] = min_hue0;
        high[0] = max_hue;
        cv::inRange(cv_image, low, high, mask1);
        cv::bitwise_or(mask0, mask1, mask);
    } else {
        cv::inRange(cv_image, low, high, mask);
    }

    // Opening
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, CV_MOP_OPEN, element);

    // find elements
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Point> polygon;

    // find largest
    int area = 0;
    cv::Rect rect;
    for (size_t i = 0; i < contours.size(); i++) {
        cv::approxPolyDP(cv::Mat(contours[i]), polygon, 3, true);
        auto tmp = cv::boundingRect(polygon);
        if (not area or (area < (tmp.width * tmp.height))) {
            rect = tmp;
            area = rect.width * rect.height;
        }
    }

#ifdef DEBUG_COLORTRACKING
    window.update(0, mask, rows, columns);
#endif  // DEBUG_COLORTRACKING

    if (contours.size()) {  // call back with center of finding
        auto const x = rect.x + (rect.width / 2);
        auto const y = rect.y + (rect.height / 2);
        callback(component_id, x, y, context);
    }
}

// free functions

template <>
bool tfv::valid<tfv::Colortracking>(TFV_Byte& min_hue, TFV_Byte& max_hue,
                                    TFV_CallbackColortrack& callback,
                                    TFV_Context& context) {
    return callback;
}

template <>
void tfv::set<tfv::Colortracking>(tfv::Colortracking* ct, TFV_Byte min_hue,
                                  TFV_Byte max_hue,
                                  TFV_CallbackColortrack callback,
                                  TFV_Context context) {
    ct->min_hue = min_hue;
    ct->max_hue = max_hue;
    ct->callback = callback;
    ct->context = context;

    if (min_hue > max_hue) {
        ct->min_hue0 = 0;
        ct->max_hue0 = COLORTRACK_MAXIMUM_HUE;
    }
}

template <>
void tfv::get<tfv::Colortracking>(tfv::Colortracking const& ct,
                                  TFV_Byte& min_hue, TFV_Byte& max_hue) {
    min_hue = ct.min_hue;
    max_hue = ct.max_hue;
}
