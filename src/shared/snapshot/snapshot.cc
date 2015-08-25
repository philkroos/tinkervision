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

DEFINE_VISION_MODULE(Snapshot)

using namespace tfv;
void Snapshot::execute(tfv::ImageData const* data, size_t width,
                       size_t height) {
    try {

        static auto counter = int(0);
        counter++;
        filename_.result =
            std::string{"Snapshot" + std::to_string(counter) + ".yuv"};

        image_.copy(data, width, height, width * height);

    } catch (...) {
        // ignore
    }
}

tfv::Result const* Snapshot::get_result(void) const {

    if (not image_.data_) {
        return nullptr;
    }

    std::ofstream ofs{filename_.result, std::ios::out | std::ios::binary};

    if (ofs.is_open()) {
        char const* data = reinterpret_cast<char const*>(image_.data_);

        ofs.write(data, image_.bytesize);
    }

    ofs.close();

    return &filename_;
}
