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

#include <vector>

#include "image.hh"
#include "tinkervision_defines.h"
#include "logger.hh"

namespace tv {

struct Result {
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    std::string result = "";
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

    virtual void execute(tv::Image const& image) = 0;

    virtual Result const* get_result(void) const { return nullptr; }
    virtual ColorSpace expected_format(void) const { return ColorSpace::NONE; }

    virtual bool has_parameter(std::string const& parameter) const {
        return false;
    }

    virtual void parameter_list(std::vector<std::string>& parameters) {
        return;
    }

    virtual bool set(std::string const& parameter, TV_Word value) {
        return false;
    }

    virtual TV_Word get(std::string const& parameter) { return 0; }

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
    ImageHeader output_header_;

    void execute(Image const& image) override final {
        get_header(image.header, output_header_);
        if (image_().header != output_header_) {
            image_.allocate(output_header_, false);
        }
        execute(image.header, image.data, image_.image());
    }

public:
    Modifier(char const* name) : TVModule(name, ModuleType::Modifier) {}
    virtual ~Modifier(void) = default;

    virtual void execute(ImageHeader const& header, ImageData const* data,
                         Image& output) = 0;

    /**
     * Called immediately before execute.
     */
    virtual void get_header(ImageHeader const& ref_header,
                            ImageHeader& output) = 0;

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

#define DECLARE_VISION_MODULE(name)        \
    extern "C" tv::TVModule* create(void); \
    extern "C" void destroy(tv::name* module);

#define DEFINE_VISION_MODULE(name)                                   \
    extern "C" tv::TVModule* create(void) { return new tv::name(); } \
    extern "C" void destroy(tv::name* module) { delete module; }

#endif
