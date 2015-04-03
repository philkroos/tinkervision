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

#ifndef EXECUTABLE_H
#define EXECUTABLE_H

#include "module.hh"

namespace tfv {
class Executable : public Module {

protected:
    Executable(TFV_Int id, std::string type) : Module(id, type) {}

public:
    virtual ~Executable(void) = default;

    virtual bool is_executable(void) const noexcept { return true; }

    // Internal part of the interface that a concrete module has to
    // implement.  The rest is defined as free function after this class
    // declaration.

    virtual ColorSpace expected_format(void) const = 0;
    virtual void execute(tfv::Image const& image) = 0;
};

// Specializations for different ColorSpace values. Just derive e.g. from
// BGRModule.

template <ColorSpace Format>
struct ExecutableWithColorSpace : public Executable {
    ExecutableWithColorSpace(TFV_Int id, std::string type)
        : Executable(id, type) {}
    virtual ~ExecutableWithColorSpace(void) = default;
    virtual ColorSpace expected_format(void) const { return Format; }
};

struct BGRModule : public ExecutableWithColorSpace<ColorSpace::BGR888> {
    virtual ~BGRModule(void) = default;
    BGRModule(TFV_Int id, std::string type)
        : ExecutableWithColorSpace(id, type) {}
};

struct RGBModule : public ExecutableWithColorSpace<ColorSpace::RGB888> {
    virtual ~RGBModule(void) = default;
    RGBModule(TFV_Int id, std::string type)
        : ExecutableWithColorSpace(id, type) {}
};

struct YUYVModule : public ExecutableWithColorSpace<ColorSpace::YUYV> {
    virtual ~YUYVModule(void) = default;
    YUYVModule(TFV_Int id, std::string type)
        : ExecutableWithColorSpace(id, type) {}
};

struct YV12Module : public ExecutableWithColorSpace<ColorSpace::YV12> {
    virtual ~YV12Module(void) = default;
    YV12Module(TFV_Int id, std::string type)
        : ExecutableWithColorSpace(id, type) {}
};
}
#endif /* EXECUTABLE_H */
