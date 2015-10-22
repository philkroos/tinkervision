/// \file downscale.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Definition of the module \c Downscale.
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

#include "downscale.hh"

DEFINE_VISION_MODULE(Downscale)

using namespace tv;

void tv::Downscale::get_header(ImageHeader const& ref, ImageHeader& output) {
    if (factor_ == 0) {
        output = ref;
    }
    auto const skip = factor_ * 2;  // downscalable by a factor of 2
    output.width = ref.width / skip;
    output.height = ref.height / skip;
    output.bytesize = output.height * output.width * 3;
    output.format = expected_format();
}

void tv::Downscale::execute(tv::ImageHeader const& header,
                            tv::ImageData const* data, tv::Image& output) {
    if (factor_ == 0) {
        std::copy_n(data, header.bytesize, output.data);
        return;
    }

    auto const channels = 3;        // receiving RGB image
    auto const skip = factor_ * 2;  // downscalable by a factor of 2
    auto source = data;
    auto target = output.data;
    auto const out_header = output.header;

    for (auto i = 0; i < out_header.height; ++i) {
        for (auto j = 0; j < out_header.width; ++j) {
            target[0] = source[0];
            target[1] = source[1];
            target[2] = source[2];
            target += channels;
            // skip the next skip-1 columns
            source += channels * skip;
        }
        // skip the next skip-1 rows, e.g. 1 row if skip is 2
        source += header.width * channels * (skip - 1);
    }

    /*
    cv::Mat cv_image(out_header.height, out_header.width, CV_8UC3);
    std::copy_n(output.data, out_header.bytesize, cv_image.data);

    cv::imshow("Downscale", cv_image);
    cv::waitKey(2);
    */
}
