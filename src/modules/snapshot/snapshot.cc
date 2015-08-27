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

#include "snapshot.hh"

#include <iostream>
#include <fstream>
#include <algorithm>

DEFINE_VISION_MODULE(Snapshot)

using namespace tfv;

Snapshot::~Snapshot(void) {
    if (image_.data) {
        delete[] image_.data;
    }
}

void Snapshot::execute(tfv::Image const& image) {
    try {

        static auto counter = int(0);
        counter++;
        filename_.result =
            std::string{"Snapshot" + std::to_string(counter) + ".yuv"};

        if (not image_.data) {
            image_.width = image.width;
            image_.height = image.height;
            image_.bytesize = image.bytesize;
            image_.data = new TFV_ImageData[image.bytesize];
        }
        std::copy_n(image.data, image.bytesize, image_.data);

    } catch (...) {
        std::cout << "Exception during Snapshotting" << std::endl;
    }
}

tfv::Result const* Snapshot::get_result(void) const {

    if (not image_.data) {
        return nullptr;
    }

    std::ofstream ofs{filename_.result, std::ios::out | std::ios::binary};

    if (ofs.is_open()) {
        char const* data = reinterpret_cast<char const*>(image_.data);

        Log("SNAPSHOT", "Wrote image as: ", filename_);
        ofs.write(data, image_.bytesize);
    }

    ofs.close();

    return &filename_;
}
