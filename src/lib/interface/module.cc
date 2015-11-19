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
tv::Module::Module(const char* name) : name_{name} {
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

    if (initialized_) {
        return false;
    }

    if (parameter_map_.find(name) != parameter_map_.cend()) {
        LogError("MODULE", name_, ": Parameter passed twice ", name);
        init_error_ = true;
        return false;
    }

    /// The parameter name must not exceed #TV_STRING_SIZE characters
    if (name.size() > TV_STRING_SIZE - 1) {
        LogError("MODULE", name_, ": Parameter name too long ", name);
        init_error_ = true;
        return false;
    }

    parameter_map_.insert({name, new Parameter(name, min, max, init)});
    parameter_names_.push_back(name);
    return true;
}

bool tv::Module::set(std::string const& parameter, int32_t value) {
    return has_parameter(parameter) and parameter_map_[parameter]->set(value);
}

bool tv::Module::get(std::string const& parameter, int32_t& value) {
    if (not has_parameter(parameter)) {
        return false;
    }

    value = parameter_map_[parameter]->get();
    return true;
}

size_t tv::Module::parameter_count(void) const { return parameter_map_.size(); }

tv::Parameter const& tv::Module::get_parameter_by_id(size_t number) const {
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

    return get_result();
}

bool tv::Module::has_result(void) const { return false; }

tv::Result const& tv::Module::get_result(void) const { return invalid_result_; }

tv::Result const& tv::Module::result(void) const {
    if (not can_have_result_ or not has_result()) {
        return invalid_result_;
    }
    return get_result();
}
