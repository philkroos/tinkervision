/// \file module.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of the class \c Module.
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

#ifndef MODULE_H
#define MODULE_H

#include <vector>
#include <unordered_map>
#include <algorithm>

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

class Module;
/// Encapsulates a parameter for use in a vision module.
/// This simplifies storage and value checking of parameters.
/// A parameter has to be constructed with the allowed range.
/// On assignment, the range will be checked. If it would be violated, no
/// assignment will be made.
class Parameter {
public:
    /// Set this parameter to a new value. Check if the limits are respected,
    /// else don't set.
    /// \param[in] The new value.
    /// \return True if setting possible.
    bool set(parameter_t value) {
        if (value < min_ or value > max_) {
            return false;
        }
        value_ = value;
        return true;
    }

    /// Return the current value of this parameter.
    /// \return value_.
    parameter_t get(void) const { return value_; }

    /// Return the name of this parameter.
    /// \return name_.
    std::string const& name(void) const { return name_; }

    /// Return the minimum allowed value.
    /// \return min_.
    parameter_t min(void) const { return min_; }

    /// Return the maximum allowed value.
    /// \return max_.
    parameter_t max(void) const { return max_; }

private:
    friend class Module;  ///< Module can create Parameter.

    /// C'tor used by Module to construct a parameter.
    /// \param[in] name Name of this parameter. Keep it short.
    /// \todo Check somewhere that length possible in the redbrick-api is
    /// respected here.
    /// \param[in] min Minimum value allowed for this parameter.
    /// \param[in] max Maximum value allowed for this parameter.
    /// \param[in] init Initialization value for this parameter.
    Parameter(std::string const& name, parameter_t min, parameter_t max,
              parameter_t init)
        : name_(name), min_(min), max_(max), value_(init) {

        if (min_ > max_) {
            LogError("PARAMETER", "Min > Max ", min_, " ", max_);
        }
        if (value_ > max_ or value_ < min_) {
            LogError("PARAMETER", "Init: ", value_);
        }
    }

    /// Never needed.
    Parameter& operator=(Parameter const& other) = delete;

    std::string const name_;  ///< Parameter name.
    parameter_t const min_;   ///< Minimum allowed value.
    parameter_t const max_;   ///< Maximum allowed value.
    parameter_t value_;       ///< Current value.
};

class Module {
private:
    std::string const name_;
    ModuleType const type_;

    std::unordered_map<std::string, Parameter*> parameter_;

protected:
    Module(const char* name, ModuleType type) : name_{name}, type_(type) {
        Log("EXECUTABLE", "Constructor for ", name);
    }

    /// Hook for modules which want to be notified about a parameter change.
    /// This will be called whenever a parameter was successfully changed.
    /// It may be usefull to store the new value reduntantly inside the actual
    /// module for faster access, if the parameter is accessed often.
    /// \note The value passed here will be in the allowed range of the
    /// registered parameter, so a deriving module can use a smaller type
    /// internally, and rely on the safeness of casting it, if possible from the
    /// limits of the parameter.
    /// \param[in] parameter The name of the changed parameter.
    /// \param[in] value New value
    virtual void value_changed(std::string const& parameter,
                               parameter_t value) {}

public:
    virtual ~Module(void) {
        Log("EXECUTABLE", "Destructor for ", name_);
        for (auto& parameter : parameter_) {
            if (parameter.second) {
                delete parameter.second;
            }
        }
    }

    std::string const& name(void) const { return name_; }
    ModuleType const& type(void) const { return type_; }

    bool has_parameter(std::string const& parameter) const {
        return parameter_.find(parameter) != parameter_.cend();
    }

    void parameter_list(std::vector<Parameter>& parameters) {
        for (auto const& parameter : parameter_) {
            auto p = parameter.second;
            parameters.push_back({p->name_, p->min_, p->max_, p->value_});
        }
    }

    void register_parameter(std::string const& name, parameter_t min,
                            parameter_t max, parameter_t init) {

        parameter_.insert({name, new Parameter(name, min, max, init)});
    }

    bool set(std::string const& parameter, parameter_t value) {
        return has_parameter(parameter) and parameter_[parameter]->set(value);
    }

    bool get(std::string const& parameter, parameter_t& value) {
        if (not has_parameter(parameter)) {
            return false;
        }

        value = parameter_[parameter]->get();
        return true;
    }

    virtual void execute(tv::Image const& image) = 0;

    virtual Result const* get_result(void) const { return nullptr; }
    virtual ColorSpace expected_format(void) const { return ColorSpace::NONE; }

    /**
     * If this module is running constantly or only on request.
     */
    virtual bool running(void) const noexcept { return true; }
};

//
// Choose what you are:
//

class Analyzer : public Module {
    using Module::Module;

    void execute(Image const& image) override final {
        execute(image.header, image.data);
    }

public:
    Analyzer(char const* name) : Module(name, ModuleType::Analyzer) {}

    virtual void execute(ImageHeader const& header, ImageData const* data) = 0;
};

class Modifier : public Module {
    using Module::Module;

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
    Modifier(char const* name) : Module(name, ModuleType::Modifier) {}
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

class Publisher : public Module {
    using Module::Module;

    void execute(Image const& image) override final {
        execute(image.header, image.data);
    }

public:
    Publisher(char const* name) : Module(name, ModuleType::Publisher) {}

    virtual void execute(ImageHeader const& header, ImageData const* data) = 0;
};
}

#define DECLARE_VISION_MODULE(name)      \
    extern "C" tv::Module* create(void); \
    extern "C" void destroy(tv::name* module);

#define DEFINE_VISION_MODULE(name)                                 \
    extern "C" tv::Module* create(void) { return new tv::name(); } \
    extern "C" void destroy(tv::name* module) { delete module; }

#endif
