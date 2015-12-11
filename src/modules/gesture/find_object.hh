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

#pragma once

#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <numeric>

#include "pixel.hh"
#include <tinkervision/image.hh>

/// Find the largest connected object.
/// Based on the method described in Watershed:
/// http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=87344, see
/// https://en.wikipedia.org/wiki/Connected-component_labeling
template <class What>
class FindObject {
public:
    using Thing = std::vector<Pixel>;
    using Acceptable = std::function<bool(Thing const& thing, What& what)>;

    FindObject(uint16_t width, uint16_t height, Acceptable acceptable);
    FindObject(uint16_t width, uint16_t height);
    ~FindObject(void);

    /// Find the first connected component with minsize connected pixels,
    /// satisfying acceptable_.
    /// This uses CurrentThing as a helper structure, where the stored thing is
    /// the connected area that is currently being labelled.
    /// The original algorithm describes ([wiki]) to push all connected
    /// foreground pixels to a stack, then pop them sequentially and push their
    /// foreground neighbors to the stack again. That way, a complete object
    /// will be labelled before the rest of the image is traversed. Here, the
    /// push/pop mechanism is solved with the looking_at variable of
    /// CurrentThing, which basically describes the number of elements pushed to
    /// thing above the currently looked at.  This is possible because this
    /// class has to return only one connected component, not label each and
    /// every.
    /// For each found large enough connected component, acceptable_ is called
    /// until it returns true, then the algorithm terminates.
    /// \param[out] what The object searched for.
    /// \param[in] minsize A possible match has to have at least that much
    /// 8-connected pixels.
    /// \param[in] image Search space.
    /// \return true If an element was found, false if no match was found.
    bool operator()(What& what, uint16_t minsize, tv::ImageData const* image);

private:
    uint16_t width_;
    uint16_t height_;
    uint16_t* labels_{nullptr};
    Acceptable acceptable_{nullptr};

    /// Helper structure to aid during object labelling.
    struct CurrentThing {
        Thing* thing;
        size_t looking_at = 0;
        inline void emplace(uint16_t x, uint16_t y, size_t idx) {
            thing->emplace_back(x, y, idx);
        }
        inline bool next(Pixel& pixel) {
            if (looking_at == count()) {
                return false;
            }
            pixel = (*thing)[looking_at++];
            return true;
        }
        inline void clean(void) {
            looking_at = 0;
            thing->clear();
        }
        inline size_t count(void) const { return thing->size(); }

        inline Thing const& it(void) const { return *thing; }

        CurrentThing(Thing& thing) : thing(&thing) {}
    };

    Thing pixel_;
    CurrentThing thing_{pixel_};
};

template <class What>
FindObject<What>::FindObject(uint16_t width, uint16_t height)
    : width_(width),
      height_(height),
      labels_(new uint16_t[width_ * height_]),
      acceptable_([](What& what, Thing const&) { return true; }) {}

template <class What>
FindObject<What>::FindObject(uint16_t width, uint16_t height,
                             Acceptable acceptable)
    : width_(width),
      height_(height),
      labels_(new uint16_t[width_ * height_]),
      acceptable_(acceptable) {}

template <class What>
FindObject<What>::~FindObject(void) {
    if (labels_) {
        delete[] labels_;
    };
}

template <class What>
bool FindObject<What>::operator()(What& what, uint16_t minsize,
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
                if (thing_.count() >= minsize and
                    acceptable_(thing_.it(), what)) {

                    // std::cout << "Found thing with " << thing_.count()
                    //           << std::endl;

                    return true;
                }

                label++;
                assert(label < std::numeric_limits<uint16_t>::max());
            }
        }
    }
    return false;
}
