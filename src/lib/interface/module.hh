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
#include <functional>

#include "image.hh"
#include "tinkervision_defines.h"
#include "logger.hh"
#include "parameter.hh"
#include "environment.hh"

#include "result.hh"

namespace tv {

class ModuleWrapper;  ///< Tinkervision-internal module representation

/// Module interface.  This class declares the only interface needed by modules
/// to be executable in the library context.  All methods to be implemented are
/// declared in the protected namespace.  The public namespace contains only
/// methods needed by the surrounding library.  The exception to this rule is
/// the parameter interface, with which modules can define the parameters they
/// want to support.  All parameters have to be registered from the constructor
/// or during initalization().  Parameter values can be get and set. They are
/// accessible by name or by a number, which corresponds to the order of their
/// construction.  Note however that the library defines additional parameters
/// which might be prepended or appended.
class Module {
protected:
    /// Construct this module with the given name.
    /// \note It is expected that this name matches the filename of the shared
    /// object (without extension).
    /// \todo Prevent construction if the name is wrong, in ModuleLoader.
    /// \param[in] name Name and identifier of this module.
    Module(const char* name, Environment const& envir);

    /// Declare the expected colorspace for input frames. This will be called
    /// once immediately after construction, before the first execute().
    /// \return The expected colorspace.
    virtual ColorSpace input_format(void) const = 0;

    /// Declare whether this module possibly produces a result.
    /// This will be called once, immediately after construction of the module,
    /// before the first execution.
    /// \sa has_result()
    /// \return False, if no result will ever be produced.
    virtual bool produces_result(void) const = 0;

    /// Declare whether this module possibly modifies the input image.
    /// If this is true, the module will be asked for the header of the output
    /// image before each execution.
    /// \sa execute()
    /// \sa get_output_image_header()
    /// \return True, iff this module produces a modified image.
    virtual bool outputs_image(void) const = 0;

    /// Construct the header of an output image for a given input image. This
    /// will be called immediately before execute().  The default implementation
    /// will return an invalid header, so if outputs_image() is true, this has
    /// to be overwritten or execute() won't be called.
    /// \param[in] input The header of the image to be expected in the next call
    /// to execute().
    /// \return A valid ImageHeader.
    virtual ImageHeader get_output_image_header(ImageHeader const& input);

    /// Possibly initialize this module.  This will be called only once after
    /// construction of this module.  The default implementation is empty.
    /// \sa initialize(), which calls this.
    virtual void init(void);

    /// Declare whether this module produced a result during the latest
    /// execute(). The default implementation will always return false.
    /// \sa execute()
    /// \return True, in case the last execute() produced a valid result.
    virtual bool has_result(void) const;

    /// Execute this module.  Header and data of both input and output image,
    /// will be passed anyways.  If the module can produce a result, it should
    /// take care to return a usefull value from has_result().  It is
    /// recommended to reset this value to false at the beginning of execute().
    /// However, it might also be fine to return the same result several times
    /// if an execution does not produce a new one, in which case has_result()
    /// must return true.  has_result(), get_result() and execute() are
    /// guaranteed to not be executed under race conditions.
    /// \param[in] input_header Header of the current camera frame.
    /// \param[in] data Data of the current camera frame.
    /// \param[in] output_header Header constructed by get_output_image_header()
    /// if outputs_image() returned true, else an invalid header.
    /// \param[out] output_data A valid pointer to a memory block of the size
    /// specified in output_header.bytesize iff this module modifies the
    /// input_image.  If this module does not produce an output image,
    /// output_data will be \c nullptr!
    virtual void execute(tv::ImageHeader const& image,
                         tv::ImageData const* data,
                         tv::ImageHeader const& output_header,
                         tv::ImageData* output_data) = 0;

    /// The module is about to be stopped.  This can be used to reset some
    /// internal state. The default implementation is empty.
    virtual void stop(void);

    /// Retrieve the result from this module.  The default implementation
    /// returns an invalid result.
    /// \sa execute()
    /// \note This will only be called if the module actually has a result, as
    /// indicated by the value of (produces_result() and has_result()).
    /// \return Latest result of this module.
    virtual Result const& get_result(void) const;

    /// Hook for modules which want to be notified about a numerical parameter
    /// change.
    /// This will be called whenever a parameter was successfully changed.
    /// It may be usefull to store the new value reduntantly inside the actual
    /// module for faster access, if the parameter is accessed often.  The
    /// default implementation does nothing.
    /// \note The value passed here will be in the allowed range of the
    /// registered parameter, so a deriving module can use a smaller type
    /// internally, and rely on the safety of a cast, if possible from the
    /// limits of the parameter.
    /// \param[in] parameter The name of the changed parameter.
    /// \param[in] value New value
    virtual void value_changed(std::string const& parameter, int32_t value) {}

    /// Hook for modules which want to be notified about a string parameter
    /// change.
    /// This will be called whenever a parameter was successfully changed.
    /// It may be usefull to store the new value reduntantly inside the actual
    /// module for faster access, if the parameter is accessed often.  The
    /// default implementation does nothing.
    /// \note The value passed here will be in the allowed range of the
    /// registered parameter, so a deriving module can use a smaller type
    /// internally, and rely on the safety of a cast, if possible from the
    /// limits of the parameter.
    /// \param[in] parameter The name of the changed parameter.
    /// \param[in] value New value
    virtual void value_changed(std::string const& parameter,
                               std::string const& value) {}

    Environment const& environment;  ///< Environment passed to the constructor

public:
    /// Default d'tor.
    virtual ~Module(void);

    using ParameterMap = std::unordered_map<std::string, Parameter*>;

    /// Add a numerice parameter to this module.  This is only possible during
    /// construction and initialization, e.g. actual modules must register their
    /// parameters in their constructor or in init().
    /// \param[in] name Name of the parameter to be constructed.  If an existing
    /// value is passed, the existing parameter will be preserved.
    /// \param[in] min Minimal allowed value.
    /// \param[in] max Maximum allowed value.
    /// \param[in] init Default value.
    /// \return True if called during initialization and no such parameter is
    /// registered yet.
    bool register_parameter(std::string const& name, int32_t min, int32_t max,
                            int32_t init);

    /// Add a string parameter to this module.  This is only possible during
    /// construction and initialization, e.g. actual modules must register their
    /// parameters in their constructor or in init().
    /// \param[in] name Name of the parameter to be constructed.  If an existing
    /// value is passed, the existing parameter will be preserved.
    /// \param[in] init Default string.
    /// \param[in] verify If provided, called with the old and new value when
    /// changing the parameter is requested, which then will only happen if the
    /// predicate evals to true.
    /// \return True if called during initialization and no such parameter is
    /// registered yet.
    bool register_parameter(
        std::string const& name, std::string const& init,
        std::function<bool(std::string const& old, std::string const& value)>
            verify = nullptr);

    /// Set the specified parameter to the given value.
    /// \param[in] parameter Name of the parameter to set.
    /// \param[in] value Value of the parameter.
    /// \return True, if the parameter exists, is numeric, and value is its the
    /// min/max range
    bool set(std::string const& parameter, int32_t value);

    /// Set the specified string parameter to the given value.
    /// \param[in] parameter Name of the parameter to set.
    /// \param[in] value Value of the parameter.
    /// \return True, if the parameter exists and is a string type.
    bool set(std::string const& parameter, std::string const& value);

    /// Get the value of the specified parameter.
    /// \param[in] parameter Name of the parameter to retrieve.
    /// \param[out] value Current value of the parameter.
    /// \return False if the specified parameter is not registered or is a
    /// string type.
    bool get(std::string const& parameter, int32_t& value) const;

    /// Get the value of the specified parameter.
    /// \param[in] parameter Name of the parameter to retrieve.
    /// \param[out] value Current value of the parameter.
    /// \return False if the specified parameter is not registered or is a
    /// numerical type.
    bool get(std::string const& parameter, std::string& value) const;

    /// Get the number of parameters registered for this module.
    /// \return Size of the parameter_map_.
    size_t parameter_count(void) const;

    /// Get a specific parameter by its number in the parameter_map_.
    /// The parameter names are stored reduntantly in a vector, to
    /// allow access by number.
    /// \param[in] number Number in range [0, parameter_count()].
    /// \return The parameter, if it exists, else the last one.
    Parameter const& get_parameter_by_number(size_t number) const;

    /// Initialize this module. This will be called immediately after
    /// construction and initializes the internal state.
    /// \return False, if the module could not be constructed or initialized
    /// correctly.
    bool initialize(void);

    /// Retrieve the name of this module.  This equals the filename of the
    /// shared object.
    std::string const& name(void) const { return name_; }

    /// Check whether this module supports a certain parameter.
    /// \param[in] parameter Name of the parameter.
    /// \return True, if the parameter exists.
    bool has_parameter(std::string const& parameter) const;

    /// Access the map of parameters.
    /// \return The mapping of parametername-Parameter.
    ParameterMap const& parameter_map(void) { return parameter_map_; }

    /// Execute this module with the current camera frame.
    /// \param[in] image Current frame.
    /// \return A valid result if the module provides one.
    Result const& execute(tv::Image const& image);

    /// Get the modified image.  If this module outputs_image(), the result of
    /// execute() will be available here.
    /// \todo Can't this be const?
    /// \return output_image_.
    Image const& modified_image(void) { return output_image_.image(); }

    /// Access the expected colorspace.
    /// \return expected_format_, as defined by input_format().
    ColorSpace expected_format(void) const { return expected_format_; }

    /// Declare whether this module produces a result.
    /// \return can_have_result_, as set by produces_result().
    bool can_have_result(void) const { return can_have_result_; }

    /// Retrieve the latest result from this module.
    /// \return get_result().
    Result const& result(void) const;

private:
    friend class ModuleWrapper;

    std::string const name_;  ///< Name of this module, c'tor parameter.

    ParameterMap parameter_map_;                ///< Available parameters.
    std::vector<std::string> parameter_names_;  ///< Provides counted access.

    ImageAllocator output_image_{"Module"};  ///< Output image, possibly unused.
    ImageHeader
        output_image_header_;     ///< Header used to construct output_image_.
    ColorSpace expected_format_;  ///< Colorspace expected for the input image.

    bool outputs_image_{false};    ///< This module modifies the image?
    bool can_have_result_{false};  ///< This module produces a result?
    bool initialized_{false};      ///< This module has been initialized?
    bool init_error_{false};       ///< An error occured during construction?

    Result invalid_result_;  ///< Default result.

    template <typename T, typename... Args>
    bool register_parameter_typed(std::string const& name, Args... args);
    template <typename T>
    bool set_parameter(std::string const& parameter, T const& value);
    template <typename T>
    bool get_parameter(std::string const& parameter, T& value) const;
};
}

#define DECLARE_VISION_MODULE(name)                              \
    extern "C" tv::Module* create(tv::Environment const& envir); \
    extern "C" void destroy(tv::name* module);

#define DEFINE_VISION_MODULE(name)                                \
    extern "C" tv::Module* create(tv::Environment const& envir) { \
        return new tv::name(envir);                               \
    }                                                             \
    extern "C" void destroy(tv::name* module) { delete module; }

#endif
