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

#include "module.hh"  // interface

#include <array>
#include <string>

#include "filesystem.hh"  // check if set path is correctly

namespace tv {
/// Save the current image to disk.
class Snapshot : public Module {

public:
    Snapshot(Environment const& envir) : Module("snapshot", envir) {}
    ~Snapshot(void) override;

protected:
    void execute(tv::ImageHeader const& header, tv::ImageData const* data,
                 tv::ImageHeader const&, tv::ImageData*) override final;

    tv::ColorSpace input_format(void) const override {
        return format_ == "yv12" ? tv::ColorSpace::YV12
                                 : tv::ColorSpace::BGR888;
    }

    void init(void) override;

    tv::Result const& get_result(void) const;

    bool has_result(void) const override final { return have_snapped_; }

    bool produces_result(void) const override final { return true; }

    bool outputs_image(void) const override final { return false; }

    void value_changed(std::string const& parameter,
                       std::string const& value) override final;

private:
    tv::Result mutable filename_;
    tv::Image image_{};
    bool have_snapped_{false};

    std::string path_{"/tmp/"};
    std::string prefix_{"tv-snap"};
    std::string format_{"jpg"};

    /// \todo Check if all of these formats are supported on the platform,
    /// probably during make.
    std::array<std::string, 8> supported_formats_{
        {"yv12", "pgm", "bmp", "png", "jpg", "jpeg", "tiff",
         "tif"}};  ///< Supported formats to save frames. Internal
    /// format yuv + see OpenCV-doc for imread.

    bool format_supported(std::string const& format) const;
};
}
DECLARE_VISION_MODULE(Snapshot)

#endif
