/// \file detect.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Definition of \c Detect.
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

#include "detect.hh"

#include <cmath>
#include <iostream>
#include <algorithm>

Detect::Detect(void)
    : history_(30),
      available_(0),
      handsize_(200),
      threshold_(50),
      detecting_{false} {}

Detect::~Detect(void) {
    if (background_) {
        delete background_;
    }

    if (foreground_) {
        delete foreground_;
    }

    if (input_) {
        delete[] input_;
    }
}

void Detect::init(uint16_t width, uint16_t height) {
    framewidth_ = width;
    frameheight_ = height;
    bytesize_ = width * height;

    if (find_) {
        delete find_;
    }
    find_ = new FindObject<Hand>(
        width, height, [this](FindObject<Hand>::Thing const& thing,
                              Hand& hand) { return acceptor_(thing, hand); });

    if (background_) {
        delete[] background_;
        background_ = nullptr;
    }
    if (foreground_) {
        delete[] foreground_;
        foreground_ = nullptr;
    }
    if (input_) {
        delete[] input_;
        input_ = nullptr;
    }
    if (refined_) {
        delete[] refined_;
        refined_ = nullptr;
    }
    detecting_ = false;
}

void Detect::set_fg_threshold(uint8_t value) { threshold_ = value; }

void Detect::set_hand_size(uint16_t size) { handsize_ = size; }

bool Detect::set_history_size(size_t size) {
    if (available_) {  // only possible until first get_hand() call
        return false;
    }
    history_ = size;
    return true;
}

bool Detect::get_hand(ImageData const* data, Hand& hand,
                      ImageData** foreground) {
    if (not background_) {
        background_ = new int32_t[bytesize_];
    }
    if (not foreground_) {
        foreground_ = new ImageData[bytesize_];
    }
    if (not input_) {
        input_ = new ImageData[bytesize_];
    }
    if (not refined_) {
        refined_ = new ImageData[bytesize_]{0};
    }

    *foreground = foreground_;

    if (available_ < history_) {  // build frames for background detection
        assert(background_);
        auto bgr = data;
        for (size_t i = 0; i < bytesize_; ++i) {
            background_[i] =
                background_[i] +
                static_cast<int32_t>(std::round(
                    0.114 * bgr[2] + 0.587 * bgr[1] + 0.299 * bgr[0]));
            bgr += 3;
        }
        available_++;
        std::cout << available_ << " available." << std::endl;
        return false;
    }

    else if (not detecting_) {
        for (size_t i = 0; i < bytesize_; ++i) {
            background_[i] /= history_;
        }
        detecting_ = true;
    }

    auto bgr = data;
    for (size_t i = 0; i < bytesize_; ++i) {
        input_[i] = static_cast<uint8_t>(
            std::round(0.114 * bgr[2] + 0.587 * bgr[1] + 0.299 * bgr[0]));
        bgr += 3;
    }

    for (size_t i = 0; i < bytesize_; ++i) {
        foreground_[i] =
            ((std::abs(background_[i] - input_[i])) > threshold_ ? 255 : 0);
    }

    acceptor_.image = data;
    acceptor_.framewidth = framewidth_;
    if (not(*find_)(hand, handsize_, foreground_)) {
        return false;
    }

    return true;
}
