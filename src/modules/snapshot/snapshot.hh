/// \file snapshot.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of the module \c Snapshot.
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

#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include "tinkervision/module.hh"

namespace tv {
/// Save the current image to disk.
/// \todo Allow parameterizing this and other modules with strings..?
class Snapshot : public Module {

public:
    Snapshot(void) : Module("snapshot") {}
    ~Snapshot(void) override;

protected:
    void execute(tv::ImageHeader const& header, tv::ImageData const* data,
                 tv::ImageHeader const&, tv::ImageData*) override final;

    tv::ColorSpace input_format(void) const override {
        return tv::ColorSpace::YV12;
    }

    tv::Result const& get_result(void) const;

    bool has_result(void) const override final { return have_snapped_; }

    bool produces_result(void) const override final { return true; }

    bool outputs_image(void) const override final { return false; }

private:
    tv::Result filename_;
    tv::Image image_{};
    bool have_snapped_{false};
};
}
DECLARE_VISION_MODULE(Snapshot)

#endif
