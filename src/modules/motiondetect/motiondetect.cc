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

#include "motiondetect.hh"

// member functions

void tfv::Motiondetect::execute(tfv::Image const& image) {
    cv::Mat frame(image.height, image.width, CV_8UC3, image.data);

    cv::Mat foreground;

    background_subtractor_.operator()(frame, foreground);

    cv::erode(foreground, foreground, cv::Mat());
    cv::dilate(foreground, foreground, cv::Mat());

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(foreground, contours, CV_RETR_EXTERNAL,
                     CV_CHAIN_APPROX_NONE);

    if (framecounter_++ >
        history_) {  // ignore the first frames to adjust to surroundings

        if (contours.size() > min_contour_count_) {
            std::vector<cv::Point> all_points;
            for (auto const& points : contours) {
                for (auto const& point : points) {
                    all_points.push_back(point);
                }
            }

            auto rect = cv::boundingRect(all_points);
            callback_(static_cast<TFV_Id>(module_id_), rect.tl().x, rect.tl().y,
                      rect.br().x, rect.br().y, context_);
        }
    }
}

// free functions

template <>
bool tfv::valid<tfv::Motiondetect>(TFV_CallbackMotiondetect& callback,
                                   TFV_Context& context) {

    // nothing to set in this module so good if callback defined
    return callback;
}

template <>
void tfv::set<tfv::Motiondetect>(tfv::Motiondetect* md,
                                 TFV_CallbackMotiondetect callback,
                                 TFV_Context context) {
    md->callback_ = callback;
    md->context_ = context;
}
