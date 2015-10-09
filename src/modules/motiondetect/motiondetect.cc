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

DEFINE_VISION_MODULE(Motiondetect)

void tv::Motiondetect::execute(tv::ImageHeader const& header,
                               ImageData const* data) {

    // Not modifying the image data in this method, so a cast is fine.
    // Thereby the original data can be used to initialize
    // the OpenCV image and no copy is necessary.
    cv::Mat frame(header.height, header.width, CV_8UC3,
                  const_cast<ImageData*>(data));
    // std::copy_n(image.data, header.bytesize, frame.data);

    cv::Mat foreground;

    background_subtractor_.operator()(frame, foreground);

    cv::erode(foreground, foreground, cv::Mat());
    cv::dilate(foreground, foreground, cv::Mat());

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(foreground, contours, CV_RETR_EXTERNAL,
                     CV_CHAIN_APPROX_NONE);

    if (framecounter_++ >
        history_) {  // ignore the first frames to adjust to surroundings

        results_ = contours.size() > min_contour_count_;
        if (results_) {

            std::vector<cv::Point> all_points;
            for (auto const& points : contours) {
                for (auto const& point : points) {
                    all_points.push_back(point);
                }
            }

            auto rect = cv::boundingRect(all_points);
            rect_around_motion_.x = rect.x;
            rect_around_motion_.y = rect.y;
            rect_around_motion_.width = rect.width;
            rect_around_motion_.height = rect.height;
        }
    }
}
