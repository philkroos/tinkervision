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

#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include "tinkervision/module.hh"

namespace tv {
class Snapshot : public Publisher {

public:
    Snapshot(void) : Publisher("Snapshot") {}
    ~Snapshot(void) override;

    void execute(ImageHeader const& header,
                 ImageData const* data) override final;

    tv::ColorSpace expected_format(void) const override {
        return tv::ColorSpace::YV12;
    }

    tv::Result const* get_result(void) const;

private:
    tv::Result filename_;
    tv::Image image_{};
};
}
DECLARE_VISION_MODULE(Snapshot)

#endif
