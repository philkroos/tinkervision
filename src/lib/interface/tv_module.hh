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

enum class ModuleType : uint8_t {
    Modifier,
    Analyzer,
    Publisher,
};

class TVModule {
private:
    std::string const name_;
    ModuleType const type_;

protected:
    TVModule(const char* name, ModuleType type) : name_{name}, type_(type) {
        Log("EXECUTABLE", "Constructor for ", name);
    }

public:
    virtual ~TVModule(void) { Log("EXECUTABLE", "Destructor for ", name_); }

    std::string const& name(void) const { return name_; }
    ModuleType const& type(void) const { return type_; }

    virtual bool init(tfv::ImageHeader const& ref_header) { return true; }

    virtual void execute(tfv::Image const& image) = 0;

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

//
// Choose what you are:
//

class Analyzer : public TVModule {
    using TVModule::TVModule;

    void execute(Image const& image) override final {
        execute(image.header, image.data);
    }

public:
    Analyzer(char const* name) : TVModule(name, ModuleType::Analyzer) {}

    virtual void execute(ImageHeader const& header, ImageData const* data) = 0;
};

class Modifier : public TVModule {
    using TVModule::TVModule;

    ImageAllocator image_;
    ImageHeader ref_header_;

    void execute(Image const& image) override final {
        execute(image.header, image.data, image_.image());
    }

    bool init(tfv::ImageHeader const& ref_header) override final {
        assert(ref_header.format == expected_format());

        ref_header_ = ref_header;  // needing this in set
        ImageHeader header;
        if (not initialize(ref_header, header) or not header) {
            return false;
        }
        return image_.allocate(header, false);
    }

public:
    Modifier(char const* name) : TVModule(name, ModuleType::Modifier) {}
    virtual ~Modifier(void) = default;

    virtual void execute(ImageHeader const& header, ImageData const* data,
                         Image& output) = 0;

    virtual bool set(std::string const& parameter, TFV_Word value) {
        return set(parameter, value) and init(ref_header_);
    }

    virtual bool initialize(ImageHeader const& ref, ImageHeader& output) = 0;

    Image const& modified_image(void) { return image_.image(); }
};

class Publisher : public TVModule {
    using TVModule::TVModule;

    void execute(Image const& image) override final {
        execute(image.header, image.data);
    }

public:
    Publisher(char const* name) : TVModule(name, ModuleType::Publisher) {}

    virtual void execute(ImageHeader const& header, ImageData const* data) = 0;
};
}

#define DECLARE_VISION_MODULE(name)         \
    extern "C" tfv::TVModule* create(void); \
    extern "C" void destroy(tfv::name* module);

#define DEFINE_VISION_MODULE(name)                                     \
    extern "C" tfv::TVModule* create(void) { return new tfv::name(); } \
    extern "C" void destroy(tfv::name* module) { delete module; }

#endif
