/// \file module_wrapper.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Defines the class ModuleWrapper.
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

#include "module_wrapper.hh"

#include <cstring>

void tv::ModuleWrapper::execute(tv::Image const& image) {
    static decltype(period_) current{0};

    /// Execute the module if period_ is not 0 and the module is scheduled to
    /// run in this cycle, decided by comparing the value of period_ with an
    /// internal counter.
    if (period_ and ++current >= period_) {
        current = 0;
        auto result = &tv_module_->execute(image);

        if (callbacks_enabled_ and cb_ and result) {
            Log("MODULE_WRAPPER", "Callback for ", module_id_, " - ", name());
            TV_ModuleResult cresult = {result->x, result->y, result->width,
                                       result->height};
            std::strncpy(cresult.string, result->result.c_str(),
                         TV_STRING_SIZE - 1);
            cresult.string[TV_STRING_SIZE - 1] = '\0';
            cb_(static_cast<int8_t>(module_id_), cresult, nullptr);
        }
    }
}

tv::ColorSpace tv::ModuleWrapper::expected_format(void) const {
    return tv_module_->expected_format();
}

std::string tv::ModuleWrapper::name(void) const { return tv_module_->name(); }

void tv::ModuleWrapper::get_parameters_list(
    std::vector<Parameter const*>& parameters) const {
    parameters.clear();

    auto map = tv_module_->parameter_map();
    for (auto const& parameter : map) {
        auto p = parameter.second;
        parameters.push_back(p);
    }
}

bool tv::ModuleWrapper::has_parameter(std::string const& parameter) const {
    /// Some parameters are supported by all modules.
    return tv_module_->has_parameter(parameter);
}

bool tv::ModuleWrapper::set_parameter(std::string const& parameter,
                                      int32_t value) {
    auto result = tv_module_->set(parameter, value);

    if (result and parameter == "period") {  // save this for faster access
        period_ = value;
    }

    return result;
}

bool tv::ModuleWrapper::set_parameter(std::string const& parameter,
                                      std::string const& value) {
    return tv_module_->set(parameter, value);
}

tv::Result const& tv::ModuleWrapper::result(void) const {
    return tv_module_->result();
}
