/// \file grayfilter.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Implementation of the module \c Grayfilter.
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

#include "grayfilter.hh"

#include <opencv2/opencv.hpp>

DEFINE_VISION_MODULE(Grayfilter)

void tv::Grayfilter::execute(tv::ImageHeader const& header,
                             tv::ImageData const* data, tv::Image& output) {

    cv::Mat cv_image(header.height, header.width, CV_8UC3, (void*)data);
    cv::cvtColor(cv_image, cv_image, CV_BGR2GRAY);

    auto gray = output.data;
    for (auto i = 0; i < header.width * header.height; ++i) {
        *gray = *(gray + 1) = *(gray + 2) = cv_image.data[i];
        gray += 3;
    }
}

void tv::Grayfilter::get_header(ImageHeader const& ref, ImageHeader& output) {
    output = ref;
}
