/// \file environment.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Defines the class Environment.
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

#include "environment.hh"

#include "tinkervision_defines.h"
#include "filesystem.hh"

#ifdef WITH_PYTHON
tv::Environment::Environment(void) noexcept(noexcept(std::string()) and
                                            noexcept(Python())) {}
#else
tv::Environment::Environment(void) noexcept(noexcept(std::string())) {}
#endif

std::string const& tv::Environment::system_modules_path(void) const {
    return system_modules_path_;
}

std::string const& tv::Environment::user_modules_path(void) const {
    return user_modules_path_;
}

std::string const& tv::Environment::user_data_path(void) const {
    return user_data_path_;
}

std::string const& tv::Environment::user_scripts_path(void) const {
    return user_scripts_path_;
}

std::string const& tv::Environment::user_prefix(void) const {
    return user_prefix_;
}

#ifdef WITH_PYTHON
bool tv::Environment::Python::set_path(std::string const& path) {
    return python_context_.set_path(path);
}

tv::Environment::Python& tv::Environment::Python::load(
    std::string const& scriptname) {

    script_ = scriptname;
    return *this;
}

std::string tv::Environment::Python::result(void) { return result_; }
#endif

bool tv::Environment::set_user_prefix(std::string const& path) {
    if (not is_directory(path)) {
        Log("ENVIRONMENT", "Can't set user prefix to ", path);
        return false;
    }

    // expecting a path ending with /
    auto dir = path;
    if (not(dir.back() == '/')) {
        dir.push_back('/');
    }

#ifdef WITH_PYTHON
    if (not is_directory(dir + MODULES_FOLDER) or
        not is_directory(dir + DATA_FOLDER) or
        not is_directory(dir + SCRIPTS_FOLDER) or
        not Environment::python_.set_path(dir + SCRIPTS_FOLDER)) {

        Log("ENVIRONMENT", "Can't set user prefix to ", dir);
        return false;
    }
#else
    if (not is_directory(dir + MODULES_FOLDER) or
        not is_directory(dir + DATA_FOLDER) or
        not is_directory(dir + SCRIPTS_FOLDER)) {

        Log("ENVIRONMENT", "Can't set user prefix to ", dir);
        return false;
    }
#endif

    user_prefix_ = dir;
    user_modules_path_ = dir + MODULES_FOLDER + "/";
    user_data_path_ = dir + DATA_FOLDER + "/";
    user_scripts_path_ = dir + SCRIPTS_FOLDER + "/";

    Log("ENVIRONMENT", "User prefix set to ", user_prefix_);

    return true;
}

#ifdef WITH_PYTHON
tv::Environment::Python& tv::Environment::Python::call(
    std::string const& function) {

    auto format = std::string("none");
    {
        std::lock_guard<std::mutex> py_lock(py_mutex_);
        (void)python_context_.execute_function(script_, function, result_,
                                               format, 1);
    }
    return *this;
}
#endif
