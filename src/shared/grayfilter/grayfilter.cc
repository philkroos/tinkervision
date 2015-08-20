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

#include "grayfilter.hh"

DEFINE_VISION_MODULE(Grayfilter)

void tfv::Grayfilter::execute_modifying(tfv::Image& image) {
    Log("GRAYFILTER", "Executing for image ", image.timestamp);

    const auto rows = image.height;
    const auto columns = image.width;
    const auto data = image.data;

    cv::Mat cv_image(rows, columns, CV_8UC3, data);
    cv::cvtColor(cv_image, cv_image, CV_BGR2GRAY);

    image.bytesize = rows * columns;
    for (size_t i = 0; i < image.bytesize; ++i) {
        image.data[i] = cv_image.data[i];
    }

    image.format = ColorSpace::GRAY;

#ifdef DEBUG
    cv::Mat output(rows, columns, CV_8UC1, image.data);
    cv::imshow("Grayfilter", output);
    cv::waitKey(2);
#endif
}
