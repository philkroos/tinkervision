#include "detect.hh"

#include <cmath>
#include <iostream>

Detect::Detect(void)
    : history_(30), available_(0), threshold_(50), detecting_{false} {}

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

void Detect::init(size_t width, size_t height) {
    framewidth_ = width;
    frameheight_ = height;
    bytesize_ = width * height;

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
    detecting_ = false;
}

void Detect::set_fg_threshold(uint8_t value) { threshold_ = value; }

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
        background_ = new uint16_t[bytesize_];
    }

    if (available_ < history_) {  // build frames for background detection
        assert(background_);
        auto bgr = data;
        for (size_t i = 0; i < bytesize_; ++i) {
            background_[i] =
                background_[i] +
                static_cast<uint16_t>(std::round(
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

    std::cout << "Detecting foreground" << std::endl;
    auto bgr = data;
    if (not input_) {
        input_ = new ImageData[bytesize_];
    }

    for (size_t i = 0; i < bytesize_; ++i) {
        input_[i] = static_cast<uint8_t>(
            std::round(0.114 * bgr[2] + 0.587 * bgr[1] + 0.299 * bgr[0]));
        bgr += 3;
    }

    // detect foreground
    if (not foreground_) {
        foreground_ = new ImageData[bytesize_];
    }
    for (size_t i = 0; i < bytesize_; ++i) {
        foreground_[i] =
            ((std::abs(background_[i] - input_[i])) > threshold_ ? 255 : 0);
    }
    *foreground = foreground_;

    return true;
}
