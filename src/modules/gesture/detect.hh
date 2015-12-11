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
#include "skin_acceptor.hh"

using namespace tv;

class Detect {
private:
    uint16_t framewidth_, frameheight_;
    size_t bytesize_;
    size_t rgb_bytesize_;
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

    SkinAcceptor* acceptor_{nullptr};
    std::string method_{""};

public:
    Detect(void);
    ~Detect(void);
    void init(uint16_t width, uint16_t height);

    void set_fg_threshold(uint8_t value);
    bool set_history_size(size_t size);
    void set_hand_size(uint16_t size);
    bool set_acceptor(std::string const& method);

    bool get_hand(ImageData const* data, Hand& hand, ImageData** foreground);
};

#endif
