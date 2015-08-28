/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014-2015 philipp.kroos@fh-bielefeld.de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "downscale.hh"

DEFINE_VISION_MODULE(Downscale)

void tfv::Downscale::execute(tfv::Image const& image, tfv::Image& out) {
    auto const channels = 3;        // receiving RGB image
    auto const skip = factor_ * 2;  // downscalable by a factor of 2

    cv::Mat c_image(image.height, image.width, CV_8UC3);
    std::copy_n(image.data, image.bytesize, c_image.data);
    cv::imshow("Downscale0", c_image);
    cv::waitKey(2);

    auto source = image.data;
    auto target = out.data;

    out.width = image.width / skip;
    out.height = image.height / skip;
    out.bytesize = image.bytesize / (skip * 2);

    for (auto i = 0; i < out.height; ++i) {
        for (auto j = 0; j < out.width; ++j) {
            target[0] = source[0];
            target[1] = source[1];
            target[2] = source[2];
            target += channels;
            // skip the next skip-1 columns
            source += channels * skip;
        }
        // skip the next skip-1 rows, e.g. 1 row if skip is 2
        source += image.width * channels * (skip - 1);
    }

    cv::Mat cv_image(out.height, out.width, CV_8UC3);
    std::copy_n(out.data, out.bytesize, cv_image.data);

    cv::imshow("Downscale", cv_image);
    cv::waitKey(2);
}
