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

#ifndef RECORD_H
#define RECORD_H

#include <iostream>
#include <fstream>

#include "executable.hh"

namespace tfv {

struct Snapshot : public YV12Module {

    explicit Snapshot(TFV_Int id) : YV12Module(id, "Snapshot") {}

    virtual void execute(tfv::Image const& image) {
        try {
            const auto file =
                std::string{"Snapshot" + std::to_string(module_id_) + "_at_" +
                            std::to_string(image.timestamp) + ".yuv"};

            std::ofstream ofs{file, std::ios::out | std::ios::binary};

            if (ofs.is_open()) {
                char const* data = reinterpret_cast<char const*>(image.data);

                ofs.write(data, image.bytesize);
            }

            ofs.close();

        } catch (...) {
            // ignore
        }

        // remove itself from execution loop
        Module::mark_for_removal();
    }
};

template <>
bool valid<Snapshot>(void) {
    return true;
}
}

#endif
