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
                    cv::imshow("Foreground", fg);
                    cv::waitKey(20);
                }

                if (labels) {
                    cv::Mat lb(header.height, header.width, CV_8UC1,
                               (void*)labels);
                    cv::imshow("Hand", lb);
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
}
