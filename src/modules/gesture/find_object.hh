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

#ifndef FIND_OBJECT_H
#define FIND_OBJECT_H

#include <vector>
#include <functional>

#include "pixel.hh"
#include <tinkervision/image.hh>

/// Find the largest connected object.
/// Based on the method described in Watershed:
/// http://ieeexplore.ieee.org/xpl/articleDetails.jsp?arnumber=87344, see
/// https://en.wikipedia.org/wiki/Connected-component_labeling
class FindObject {
public:
    using Thing = std::vector<Pixel>;
    using Result = std::vector<Pixel>;
    using Acceptable = std::function<bool(Thing const& thing)>;

    FindObject(uint16_t width, uint16_t height, Acceptable acceptable);
    FindObject(uint16_t width, uint16_t height);
    ~FindObject(void);

    bool operator()(Result&, uint16_t minsize, tv::ImageData const* image);

private:
    uint16_t width_;
    uint16_t height_;
    uint16_t* labels_{nullptr};
    Acceptable acceptable_{nullptr};

    struct CurrentThing {
        Thing* thing = nullptr;
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

#endif
