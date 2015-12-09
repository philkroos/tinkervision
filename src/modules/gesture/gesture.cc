/// \file gesture.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Definition of the module \c Gesture.
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

#include <opencv2/opencv.hpp>
#include <iostream>

#include "gesture.hh"

DEFINE_VISION_MODULE(Gesture)

static void auto_canny(cv::Mat& image) {

    // Calculate histogram
    auto hist_size = int(256);
    float const range[] = {0.0f, 255.0f};
    float const* ranges[] = {range};

    cv::MatND histogram;
    cv::calcHist(&image, 1, 0, cv::Mat(), histogram, 1, &hist_size, ranges);

    auto count = int(0);
    auto median = int(0);
    auto values = image.rows * image.cols;
    for (auto i = 0; i < histogram.rows; ++i) {
        count += histogram.at<int>(i, 0);
        if (count >= values) {
            median = static_cast<int>(i);
            break;
        }
    }

    auto thresh_low = static_cast<int>(0.5 * median);
    auto thresh_high = static_cast<int>(1.5 * median);
    auto kernel_size = int(3);  // good?
    std::cout << "Canny with " << thresh_low << "," << thresh_high << std::endl;

    cv::Canny(image, image, thresh_low, thresh_high, kernel_size);
}

void tv::Gesture::value_changed(std::string const& parameter, int32_t value) {
    if (parameter == "bg-history") {
        detect_.set_history_size(static_cast<size_t>(value));
    } else if (parameter == "fg-threshold") {
        detect_.set_fg_threshold(static_cast<uint8_t>(value));
    } else if (parameter == "min-hand-size") {
        detect_.set_hand_size(static_cast<uint16_t>(value));
    }
}

void tv::Gesture::execute(tv::ImageHeader const& header, ImageData const* data,
                          tv::ImageHeader const&, ImageData*) {

    if (header != ref_header_) {
        // different image size needs to retrigger detection
        state_ = State::Initial;
        ref_header_ = header;
    }

    switch (state_) {
        case State::Initial:
            detect_.init(static_cast<uint16_t>(header.width),
                         static_cast<uint16_t>(header.height));
            state_ = State::Detect;
            break;
        case State::Detect: {
            ImageData* foreground{nullptr};
            ImageData* labels{nullptr};
            if (detect_.get_hand(data, hand_, &foreground, &labels)) {
                if (foreground) {
                    cv::Mat fg(header.height, header.width, CV_8UC1,
                               (void*)foreground);
                    cv::imshow("Canny", fg);
                    cv::waitKey(20);
                }

                if (labels) {
                    cv::Mat lb(header.height, header.width, CV_8UC1,
                               (void*)labels);
                    cv::imshow("Gesture", lb);
                    cv::waitKey(20);
                }

                // Log("Gesture", "Hand detected at ", hand_.center);
            }
            break;
        }
        case State::Track:
            Log("Gesture", "Tracking");
            break;
        case State::Match:
            Log("Gesture", "Matching");
            break;
        default:
            break;
    }

    return;
    Log("GESTURE", "execute");
    cv::Mat cv_image(header.height, header.width, CV_8UC3);
    std::copy_n(data, header.bytesize, cv_image.data);

    // canny
    cv::Mat gray, edges, canny;
    canny.create(cv_image.size(), cv_image.type());
    cv::cvtColor(cv_image, gray, CV_BGR2GRAY);
    cv::blur(gray, edges, cv::Size(3, 3));
    auto_canny(edges);
    canny = cv::Scalar::all(0);
    cv_image.copyTo(canny, edges);
    cv::imshow("Canny", canny);
    cv::waitKey(50);
    // end of canny

    cv::cvtColor(cv_image, cv_image, CV_BGR2HSV);
    cv::Mat mask(header.height, header.width, CV_8UC3);

    cv::Scalar low(min_hue, min_saturation, min_value);
    cv::Scalar high(max_hue, max_saturation, max_value);

    cv::inRange(cv_image, low, high, mask);

    // Opening
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, CV_MOP_OPEN, element);
    cv::imshow("HSV", mask);
    cv::waitKey(50);

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
}
