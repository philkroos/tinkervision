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

#ifndef TV_MODULE_H
#define TV_MODULE_H

#include "image.hh"
#include "tinkervision_defines.h"
#include "logger.hh"

namespace tfv {

struct Result {
    virtual ~Result(void) = default;
};

struct StringResult : public Result {
    using callback_t = TFV_CallbackString;
    std::string result = "";
    StringResult(void) = default;
    StringResult(std::string const& s) : result(s) {}
};

struct ScalarResult : public Result {
    using callback_t = TFV_CallbackValue;
    TFV_Size scalar = 0;
    ScalarResult(void) = default;
    ScalarResult(TFV_Size i) : scalar(i) {}
};

struct PointResult : public Result {
    using callback_t = TFV_CallbackPoint;
    TFV_Size x = 0;
    TFV_Size y = 0;
    PointResult(void) = default;
    PointResult(TFV_Size x, TFV_Size y) : x(x), y(y) {}
};

struct RectangleResult : public Result {
    using callback_t = TFV_CallbackRectangle;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    RectangleResult(void) = default;
    RectangleResult(int x, int y, int width, int height)
        : x(x), y(y), width(width), height(height) {}
};

class TVModule {
private:
    std::string const name_;

public:
    explicit TVModule(const char* name) : name_{name} {
        Log("EXECUTABLE", "Constructor for ", name);
    }

    virtual ~TVModule(void) { Log("EXECUTABLE", "Destructor for ", name_); }

    const char* name(void) const { return name_.c_str(); }
    // virtual void execute(tfv::Image const& image) {
    virtual void execute(tfv::Image const& image) {
        LogError("EXECUTABLE", "execute called");
    }
    virtual void execute_modifying(tfv::Image& image) {
        LogError("EXECUTABLE", "execute_modifying called");
    }

    virtual bool modifies_image(void) const { return false; }
    virtual Result const* get_result(void) const { return nullptr; }
    virtual ColorSpace expected_format(void) const { return ColorSpace::NONE; }

    virtual bool has_parameter(std::string const& parameter) const {
        return false;
    }

    virtual bool set(std::string const& parameter, TFV_Word value) {
        return false;
    }
    virtual TFV_Word get(std::string const& parameter) { return 0; }

    /**
     * If this module is running constantly or only on request.
     */
    virtual bool running(void) const noexcept { return true; }
};
}
#define DECLARE_VISION_MODULE(name)         \
    extern "C" tfv::TVModule* create(void); \
    extern "C" void destroy(tfv::name* module);

#define DEFINE_VISION_MODULE(name)                                     \
    extern "C" tfv::TVModule* create(void) { return new tfv::name(); } \
    extern "C" void destroy(tfv::name* module) { delete module; }

#endif
