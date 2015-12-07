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

    if (labels_) {
        delete[] labels_;
    }
}

void Detect::init(uint16_t width, uint16_t height) {
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
    if (labels_) {
        delete[] labels_;
        labels_ = nullptr;
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

bool Detect::get_hand(ImageData const* data, Hand& hand, ImageData** foreground,
                      ImageData** labels) {
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

    auto bgr = data;
    if (not input_) {
        input_ = new ImageData[bytesize_];
    }
    if (not labels_) {
        labels_ = new uint16_t[bytesize_]{0};
    }
    if (not refined_) {
        refined_ = new ImageData[bytesize_]{0};
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
    objects_.clear();

    // labels_[framewidth_ + 1] = 0;  // unlabel initial pixel
    uint16_t label = 1;
    std::vector<Pixel> current_object;

    for (size_t i = 0; i < bytesize_; ++i) {
        foreground_[i] =
            ((std::abs(background_[i] - input_[i])) > threshold_ ? 255 : 0);
        labels_[i] = 0;
    }

    // https://en.wikipedia.org/wiki/Connected-component_labeling
    // Method described in Watershed:
    // http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=87344
    for (uint16_t i = 1; i < frameheight_ - 1; ++i) {
        auto const offset = i * framewidth_;
        for (uint16_t j = 1; j < framewidth_ - 1; ++j) {

            size_t const idx = offset + j;

            if (not foreground_[idx]) {
                continue;
            }

            if (foreground_[idx] and not labels_[idx]) {
                labels_[idx] = label;
                objects_.emplace_back(i, j, 1, label);
                current_object.emplace_back(j, i);

                while (current_object.size()) {
                    auto const x = current_object.back().x;
                    auto const y = current_object.back().y;
                    current_object.pop_back();
                    auto const px = y * framewidth_ + x;
                    auto const right = (x == (framewidth_ - 1));
                    auto const left = (x == 0);
                    auto const top = (y == 0);
                    auto const bot = (y == (frameheight_ - 1));

                    if (not left) {
                        if (foreground_[px - 1] and not labels_[px - 1]) {
                            labels_[px - 1] = label;
                            objects_[label - 1].count++;
                            current_object.emplace_back(uint16_t(x - 1),
                                                        (uint16_t)y);

                            if (not bot and
                                foreground_[px - 1 + framewidth_] and
                                not labels_[px - 1 + framewidth_]) {
                                labels_[px - 1 + framewidth_] = label;
                                objects_[label - 1].count++;
                                current_object.emplace_back(uint16_t(x - 1),
                                                            uint16_t(y + 1));
                            }
                            if (not top and
                                foreground_[px - 1 - framewidth_] and
                                not labels_[px - 1 - framewidth_]) {
                                labels_[px - 1 - framewidth_] = label;
                                objects_[label - 1].count++;
                                current_object.emplace_back(uint16_t(x - 1),
                                                            uint16_t(y - 1));
                            }
                        }
                    }
                    if (not top and foreground_[px - framewidth_] and
                        not labels_[px - framewidth_]) {
                        labels_[px - framewidth_] = label;
                        objects_[label - 1].count++;
                        current_object.emplace_back(uint16_t(x),
                                                    uint16_t(y - 1));
                    }
                    if (not bot and foreground_[px + framewidth_] and
                        not labels_[px + framewidth_]) {
                        labels_[px + framewidth_] = label;
                        objects_[label - 1].count++;
                        current_object.emplace_back(uint16_t(x),
                                                    uint16_t(y + 1));
                    }
                    if (not right) {
                        if (foreground_[px + 1] and not labels_[px + 1]) {
                            objects_[label - 1].count++;
                            current_object.emplace_back(uint16_t(x + 1),
                                                        uint16_t(y));

                            if (not bot and
                                foreground_[px + 1 + framewidth_] and
                                not labels_[px + 1 + framewidth_]) {
                                labels_[px + 1 + framewidth_] = label;
                                objects_[label - 1].count++;
                                current_object.emplace_back(uint16_t(x + 1),
                                                            uint16_t(y + 1));
                            }
                            if (not top and
                                foreground_[px + 1 - framewidth_] and
                                not labels_[px + 1 - framewidth_]) {
                                labels_[px + 1 - framewidth_] = label;
                                objects_[label - 1].count++;
                                current_object.emplace_back(uint16_t(x + 1),
                                                            uint16_t(y - 1));
                            }
                        }
                    }
                }
                label++;
            }
        }
    }

    if (not objects_.size()) return true;

    // Expecting the hand to be the largest moving object
    Object* most = &objects_[0];
    for (auto& o : objects_) {
        if (o.count > most->count) {
            most = &o;
        }
    }

    auto const value = most->label;
    if (most->count > handsize_) {
        size_t count = 0;
        for (size_t i = 0; i < bytesize_; ++i) {
            if (labels_[i] == value) {
                count++;
                refined_[i] = 255;
            } else {
                refined_[i] = 0;
            }
        }
    }

    *foreground = foreground_;
    *labels = refined_;

    return true;
}
