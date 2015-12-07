/// \file module.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Implementation of the interface \c Module.
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

#include "module.hh"

tv::Module::Module(const char* name, tv::Environment const& envir)
    : environment(envir), name_{name} {
    Log("EXECUTABLE", "Constructor for ", name);
}

tv::Module::~Module(void) {
    Log("EXECUTABLE", "Destructor for ", name_);
    for (auto& parameter : parameter_map_) {
        if (parameter.second) {
            delete parameter.second;
        }
    }
}

bool tv::Module::initialize(void) {
    Log("MODULE", "Initializing ", name_);

    if (init_error_) {
        LogError("MODULE", "Initializing failed during construction");
        return false;
    }

    /// - Define whether this module modifies the input image.
    outputs_image_ = outputs_image();
    /// - Define whether this module might produce a result.
    can_have_result_ = produces_result();
    /// - Define the expected format of input camera frames.
    expected_format_ = input_format();

    /// - Call init(), in case the module wants to do more initialization work.
    init();

    Log("MODULE", " ", outputs_image_, can_have_result_, expected_format_);
    initialized_ = true;

    return true;
}

void tv::Module::init(void) {}

bool tv::Module::has_parameter(std::string const& parameter) const {
    return parameter_map_.find(parameter) != parameter_map_.cend();
}

bool tv::Module::register_parameter(std::string const& name, int32_t min,
                                    int32_t max, int32_t init) {
    if (max < min or init < min or init > max) {
        LogError("MODULE", name_, ": Invalid values: ", min, " ", max, " ",
                 init);
        init_error_ = true;
        return false;
    }
    return register_parameter_typed<NumericalParameter>(name, min, max, init);
}

bool tv::Module::register_parameter(
    std::string const& name, std::string const& init,
    std::function<bool(std::string const& old, std::string const& value)>
        verify) {

    if (init.size() >= TV_STRING_SIZE) {
        LogError("MODULE", name_, ": Parameter default value too long ", init);
        init_error_ = true;
        return false;
    }
    return register_parameter_typed<StringParameter>(name, init, verify);
}

template <typename T, typename... Args>
bool tv::Module::register_parameter_typed(std::string const& name,
                                          Args... args) {
    if (initialized_) {
        return false;
    }

    if (parameter_map_.find(name) != parameter_map_.cend()) {
        LogError("MODULE", name_, ": Parameter passed twice ", name);
        init_error_ = true;
        return false;
    }

    /// The parameter name must not exceed #TV_STRING_SIZE characters
    if (name.size() >= TV_STRING_SIZE) {
        LogError("MODULE", name_, ": Parameter name too long ", name);
        init_error_ = true;
        return false;
    }

    parameter_map_.insert({name, new T(name, args...)});
    parameter_names_.push_back(name);
    return true;
}

bool tv::Module::set(std::string const& parameter, int32_t value) {
    return set_parameter(parameter, value);
}

bool tv::Module::set(std::string const& parameter, std::string const& value) {
    return set_parameter(parameter, value);
}

template <typename T>
bool tv::Module::set_parameter(std::string const& parameter, T const& value) {
    if (has_parameter(parameter) and parameter_map_[parameter]->set(value)) {
        value_changed(parameter, value);
        return true;
    }
    return false;
}

bool tv::Module::get(std::string const& parameter, int32_t& value) const {
    return get_parameter(parameter, value);
}

bool tv::Module::get(std::string const& parameter, std::string& value) const {
    return get_parameter(parameter, value);
}

template <typename T>
bool tv::Module::get_parameter(std::string const& parameter, T& value) const {
    if (not has_parameter(parameter)) {
        return false;
    }

    return parameter_map_.at(parameter)->get(value);
}

size_t tv::Module::parameter_count(void) const { return parameter_map_.size(); }

tv::Parameter const& tv::Module::get_parameter_by_number(size_t number) const {
    // return last if out-of-range
    auto name = parameter_names_[std::min(number, parameter_count())];
    return *parameter_map_.find(name)->second;
}

tv::ImageHeader tv::Module::get_output_image_header(ImageHeader const& input) {
    return ImageHeader();
}

tv::Result const& tv::Module::execute(tv::Image const& image) {
    /// If the module declared that it outputs_image(), it will be queried for
    /// the header of the output image first.
    if (outputs_image_) {
        output_image_header_ = get_output_image_header(image.header);
        /// If no valid header is returned, execute() won't be called.
        if (not output_image_header_) {
            return invalid_result_;
        }
        /// Else, a valid output image will be allocated
        if (output_image_header_ != output_image_.header()) {
            output_image_.allocate(output_image_header_, false);
        }
    }

    /// and header and data of input and output image will be passed to execute.
    execute(image.header, image.data, output_image_.header(),
            output_image_.image().data);

    return result();
}

bool tv::Module::has_result(void) const { return false; }

tv::Result const& tv::Module::get_result(void) const { return invalid_result_; }

tv::Result const& tv::Module::result(void) const {
    return (can_have_result_ and has_result()) ? get_result() : invalid_result_;
}

void tv::Module::stop(void) {}
