#include "detect.hh"

#include <cmath>
#include <iostream>

Detect::Detect(void) : history_(50), available_(0), threshold_(50) {}

Detect::~Detect(void) {
    for (auto& i : images_) {
        if (i) {
            delete[] i;
        }
    }

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

    set_history_size(0);
    set_history_size(history_);

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
}

void Detect::set_fg_threshold(uint8_t value) { threshold_ = value; }

void Detect::set_history_size(size_t size) {
    if (size > images_.size()) {
        std::cout << "Allocating " << (size - images_.size()) << " images."
                  << std::endl;
        for (size_t i = images_.size(); i < size; ++i) {
            images_.push_back(new ImageData[bytesize_]);
        }
    } else if (size < images_.size()) {  // remove oldest frames
        std::cout << "Removing " << (images_.size() - size) << " images."
                  << std::endl;
        for (size_t i = 0, j = size; i < (images_.size() - size); ++i, ++j) {
            if (images_[i]) {
                delete images_[i];
                images_[i] = nullptr;
            }
            if (size) {
                images_[i] = images_[j];
            }
        }
    }
}

bool Detect::get_hand(ImageData const* data, Hand& hand,
                      ImageData** foreground) {
    if (available_ < history_) {  // build frames for background detection
        assert(images_[available_]);
        auto bgr = data;
        for (size_t i = 0; i < bytesize_; ++i) {
            images_[available_][i] = static_cast<uint8_t>(
                std::round(0.114 * bgr[2] + 0.587 * bgr[1] + 0.299 * bgr[0]));
            bgr += 3;
        }
        available_++;
        std::cout << available_ << " available." << std::endl;
        return false;
    }

    else if (not background_) {  // compute background image
        background_ = new uint16_t[bytesize_];

        for (size_t i = 0; i < images_.size(); ++i) {
            for (size_t j = 0; j < bytesize_; ++j) {
                background_[j] += images_[i][j];
            }
        }

        for (size_t i = 0; i < bytesize_; ++i) {
            background_[i] /= history_;
        }
        return false;
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
        foreground_[i] =  // background_[i];
            ((std::abs(background_[i] - input_[i])) > threshold_ ? 255 : 0);
    }
    *foreground = foreground_;

    return true;
}
