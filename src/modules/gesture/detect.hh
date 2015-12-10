/// \file detect.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of \c Detect, part of the gesture module.
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

#ifndef DETECT_H
#define DETECT_H

#include <vector>
#include <iostream>
#include <algorithm>

#include <opencv2/opencv.hpp>

#include <image.hh>

#include "hand.hh"
#include "find_object.hh"

using namespace tv;

class Detect {
private:
    uint16_t framewidth_, frameheight_;
    size_t bytesize_;
    size_t history_;     ///< Number of images for background calculation
    size_t available_;   ///< Currently available images, 0 < x <= history_
    uint16_t handsize_;  ///< Minimum size of connected area to be possible hand
    uint8_t threshold_;  ///< If (pixel - background) > threshold, foreground.
    bool detecting_;     ///< State

    int32_t* background_{nullptr};
    ImageData* foreground_{nullptr};
    ImageData* input_{nullptr};
    ImageData* refined_{nullptr};

    FindObject<Hand>* find_{nullptr};

    struct Acceptor {
        ImageData const* image{nullptr};
        size_t framewidth;
        bool operator()(FindObject<Hand>::Thing const& thing, Hand& hand) {
            if (not image) {
                return false;
            }

            auto minmaxx =
                std::minmax_element(thing.cbegin(), thing.cend(),
                                    [](Pixel const& lhs, Pixel const& rhs) {
                    return lhs.x < rhs.x;
                });
            auto minmaxy =
                std::minmax_element(thing.cbegin(), thing.cend(),
                                    [](Pixel const& lhs, Pixel const& rhs) {
                    return lhs.y < rhs.y;
                });

            hand.x = thing[minmaxx.first - thing.cbegin()].x;
            hand.width = thing[minmaxx.second - thing.cbegin()].x - hand.x;
            hand.y = thing[minmaxy.first - thing.cbegin()].y;
            hand.height = thing[minmaxy.second - thing.cbegin()].y - hand.y;

            uint8_t b, g, r;
            bgr_average(thing, image, b, g, r);
            std::cout << "--Average: " << (int)b << "," << (int)g << ","
                      << (int)r << std::endl;
            std::cout << "--Hand: " << hand << std::endl;

            // explicit skin region, see "A Survey on Pixel-Based Skin Color
            // Detection Techniques"
            return r > 95 and g > 40 and b > 20 and r > g and r > b and
                   std::abs(r - g) > 15 and
                   (std::max(std::max(r, g), b) - std::min(std::min(r, g), b) >
                    15);
        }
    };

    Acceptor acceptor_;

public:
    Detect(void);
    ~Detect(void);
    void init(uint16_t width, uint16_t height);

    void set_fg_threshold(uint8_t value);
    bool set_history_size(size_t size);
    void set_hand_size(uint16_t size);

    bool get_hand(ImageData const* data, Hand& hand, ImageData** foreground);
};

#endif
