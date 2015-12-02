/// \file snapshot.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Definition of the module \c Snapshot.
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

#include "snapshot.hh"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <opencv2/opencv.hpp>

DEFINE_VISION_MODULE(Snapshot)

tv::Snapshot::~Snapshot(void) {
    if (image_.data) {
        delete[] image_.data;
    }
}

void tv::Snapshot::execute(tv::ImageHeader const& header,
                           tv::ImageData const* data, tv::ImageHeader const&,
                           tv::ImageData*) {
    try {
        // if (not have_snapped_) {
        //     cv::namedWindow("Exec");
        //     cv::namedWindow("Snapshot");
        // }
        if (header.format == ColorSpace::BGR888) {
            cv::Mat image(header.height, header.width, CV_8UC3);
            std::copy_n(data, header.bytesize, image.data);

            // cv::imshow("Exec", image);
            // cv::waitKey(20);
        } else {
            Log("SNAPSHOT", "Requested ", format_, ", got", header.format);
        }

        if (not image_.data) {
            image_.header = header;
            image_.data = new uint8_t[header.bytesize];
        }
        std::copy_n(data, header.bytesize, image_.data);
        have_snapped_ = true;
    } catch (...) {
        std::cout << "Exception during Snapshotting" << std::endl;
    }
}

tv::Result const& tv::Snapshot::get_result(void) const {
    if (not image_.data) {
        LogError("SNAPSHOT", "No data");
        return filename_;
    }

    static auto counter = uint64_t(0);
    counter++;

    filename_.result = std::string{path_ + "/" + prefix_ + "_" +
                                   std::to_string(counter) + "." + format_};

    if (image_.header.format == ColorSpace::YV12) {
        std::ofstream ofs{filename_.result, std::ios::out | std::ios::binary};

        if (ofs.is_open()) {
            char const* data = reinterpret_cast<char const*>(image_.data);
            ofs.write(data, image_.header.bytesize);
        }
        ofs.close();
        Log("SNAPSHOT", "Wrote image as: ", filename_.result);
    } else if (image_.header.format == ColorSpace::BGR888) {
        cv::Mat image(image_.header.height, image_.header.width, CV_8UC3,
                      (void*)image_.data);

        cv::imwrite(filename_.result, image);
        Log("SNAPSHOT", "Wrote image as: ", filename_.result);

        // cv::imshow("Snapshot", image);
        // cv::waitKey(20);

    } else {
        LogError("SNAPSHOT", "Invalid format ", image_.header.format);
    }

    return filename_;
}

void tv::Snapshot::init(void) {
    register_parameter("prefix", prefix_);
    register_parameter(
        "format", format_,
        [this](std::string const& old_format, std::string const& new_format) {
            return format_supported(new_format);
        });
    register_parameter("path", path_, [](std::string const& old_path,
                                         std::string const& new_path) {
        return is_directory(new_path);
    });
}

bool tv::Snapshot::format_supported(std::string const& format) const {
    for (auto const& f : supported_formats_) {
        if (f == format) {
            return true;
        }
    }
    return false;
}

void tv::Snapshot::value_changed(std::string const& parameter,
                                 std::string const& value) {
    if (parameter == "format") {
        if (value != format_ and
            (value == "yv12" or format_ == "yv12")) {  // switching from manual
                                                       // write to cv::imwrite
                                                       // or reverse
            if (image_.data) {
                delete image_.data;
                image_.data = nullptr;
            }
        }
        Log("SNAPSHOT", "Selected format: ", format_);
    }

    auto& target =
        (parameter == "format" ? format_ : parameter == "path" ? path_
                                                               : prefix_);
    target = value;
}
