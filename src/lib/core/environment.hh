/// \file environment.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declares the class Environment.
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

#include <string>

#include "logger.hh"

namespace tv {

class Api;  ///< Constructing Environment

class Environment {
public:
    std::string const& system_module_path(void) { return system_module_path_; }

    std::string user_module_path(void) { return user_prefix_ + modules_dir_; }

    /// Set the user prefix, which is the root of all accessible user paths.
    /// The path has to exist and each of the paths accessible from the user_*
    /// methods of this class have to exist.
    /// \param[in] path An existing path with user write privileges.
    bool set_user_prefix(std::string const& path) {
        if (not is_directory(path)) {
            Log("ENVIRONMENT", "Can't set user prefix to ", path);
            return false;
        }

        // expecting a path ending with /
        auto dir = path;
        if (not(dir.back() == '/')) {
            dir.push_back('/');
        }

        if (not is_directory(dir + modules_dir_) or
            not is_directory(dir + frames_dir_)) {

            Log("ENVIRONMENT", "Can't set user prefix to ", dir);
            return false;
        }

        user_prefix_ = dir;
        Log("ENVIRONMENT", "User prefix set to ", dir);

        return true;
    }

private:
    std::string const system_module_path_{"/usr/lib/tinkervision/"};
    std::string const modules_dir_{"modules"};
    std::string const frames_dir_{"frames"};

    std::string user_prefix_;

    Environment(void) noexcept {}

    friend class Api;
};
}
