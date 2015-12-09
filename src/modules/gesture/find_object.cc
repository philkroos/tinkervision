/// \file find_object.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of \c FindObject, part of the gesture module.
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

#include "find_object.hh"

#include <algorithm>
#include <stack>
#include <iostream>
#include <cassert>
#include <numeric>

FindObject::FindObject(uint16_t width, uint16_t height)
    : width_(width),
      height_(height),
      labels_(new uint16_t[width_ * height_]),
      acceptable_([](Thing const& thing) { return true; }) {}

FindObject::FindObject(uint16_t width, uint16_t height, Acceptable acceptable)
    : width_(width),
      height_(height),
      labels_(new uint16_t[width_ * height_]),
      acceptable_(acceptable) {}

FindObject::~FindObject(void) {
    if (labels_) {
        delete[] labels_;
    };
}

bool FindObject::operator()(Result& result, uint16_t minsize,
                            tv::ImageData const* image) {

    std::fill_n(labels_, width_ * height_, 0);
    uint16_t label = 1;

    Pixel pixel;  // currently looking at

    auto const bborder = height_ - 1;
    auto const rborder = width_ - 1;
    for (uint16_t i = 0; i <= bborder; ++i) {
        auto const row = i * width_;

        for (uint16_t j = 0; j <= rborder; ++j) {
            size_t const idx = row + j;

            thing_.clean();
            if (image[idx] and not labels_[idx]) {
                thing_.emplace(j, i, idx);
                labels_[idx] = label;

                while (thing_.next(pixel)) {
                    auto const objx = pixel.x;
                    auto const objy = pixel.y;
                    auto const px = pixel.idx;

                    auto const rlimit = (objx == rborder ? 0 : 1);
                    auto const blimit = (objy == bborder ? 0 : 1);

                    for (int y = (objy ? -1 : 0); y <= blimit; ++y) {
                        auto const line = px + y * width_;

                        for (int x = (objx ? -1 : 0); x <= rlimit; ++x) {
                            auto const pos = line + x;

                            if (image[pos] and not labels_[pos]) {
                                thing_.emplace(objx + x, objy + y, pos);
                                labels_[pos] = label;
                            }
                        }
                    }
                }
                std::cout << "Got thing with " << thing_.count() << std::endl;
                if (thing_.count() >= minsize and acceptable_(thing_.it())) {
                    std::cout << "Found thing with " << thing_.count()
                              << std::endl;
                    result = thing_.it();
                    return true;
                }

                label++;
                assert(label < std::numeric_limits<uint16_t>::max());
            }
        }
    }
    return false;

    /*
    std::cout << "Found " << objects_.size() << " objects." << std::endl;
    if (not objects_.size()) return true;

    // Looking for the largest object
    Object* most = &objects_[0];
    for (auto& o : objects_) {
        if (o.count > most->count) {
            most = &o;
        }
    }

    std::cout << "Got " << most->count << " hits." << std::endl;
    if (most->count > minsize) {
        auto const value = most->label;

        size_t px(0);
        for (uint16_t i = 1; i < height_ - 1; ++i) {
            auto const row = i * width_;

            for (uint16_t j = 1; j < width_ - 1; ++j, px++) {
                if (labels_[row + j] == value) {
                    result.emplace_back(j, i, px);
                }
            }
        }
        return true;
    }

    return false;
    */
}
