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
    rgb_bytesize_ = bytesize_ * 3;

    if (find_) {
        delete find_;
    }
    find_ = new FindObject<Hand>(
        width, height,
        [this](FindObject<Hand>::Thing const& thing,
               Hand& hand) { return (*acceptor_)(thing, hand); });

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

    if (not acceptor_) {
        set_acceptor("explicit");
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

bool Detect::set_acceptor(std::string const& method) {
    if (method == method_) {
        return true;
    }
    SkinAcceptor* new_acceptor{nullptr};
    if (method == "explicit") {
        new_acceptor = new ExplicitSkinRegionAcceptor;
    }

    if (not new_acceptor) {
        return false;
    }

    auto old_acceptor = acceptor_;
    acceptor_ = new_acceptor;

    if (old_acceptor) {
        delete old_acceptor;
    }
    return true;
}

bool Detect::get_hand(ImageData const* data, Hand& hand,
                      ImageData** foreground) {
    if (not background_) {
        background_ = new int32_t[rgb_bytesize_];
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
        for (size_t i = 0; i < rgb_bytesize_; ++i) {
            background_[i] += static_cast<int32_t>(bgr[i]);
        }
        available_++;
        std::cout << available_ << " available." << std::endl;
        return false;
    }

    else if (not detecting_) {
        for (size_t i = 0; i < rgb_bytesize_; ++i) {
            background_[i] /= history_;
        }
        detecting_ = true;
    }

    auto rgb_back = background_;
    auto rgb = data;
    /// Uses the similarity metric from "View-based Detection and Analysis of
    /// Periodic Motion" [Cutler/Davis]
    uint16_t sim;
    for (size_t i = 0; i < bytesize_; ++i) {
        sim = std::abs((int)*rgb++ - (int)*rgb_back++);
        sim += std::abs((int)*rgb++ - (int)*rgb_back++);
        sim += std::abs((int)*rgb++ - (int)*rgb_back++);
        foreground_[i] = sim > threshold_ ? 255 : 0;
    }

    acceptor_->image = data;
    acceptor_->framewidth = framewidth_;
    if (not(*find_)(hand, handsize_, foreground_)) {
        return false;
    }

    return true;
}
