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
#include "filesystem.hh"
#include "python_context.hh"

namespace tv {

class Api;  ///< Constructing Environment

class Environment {
public:
    /// Get the path where core vision modules are installed.
    /// \return system_module_path_.
    std::string const& system_modules_path(void) const;

    /// Get the path where additional vision modules are installed.
    /// \return user_prefix_/modules_dir_.
    std::string const& user_modules_path(void) const;

    /// Get the path where modules should save frames.
    /// \return user_prefix_/frames_dir_.
    std::string const& user_frames_path(void) const;

    /// Get the path where python scripts can be loaded from.
    /// \return user_prefix_/scripts_dir_.
    std::string const& user_scripts_path(void) const;

    /// Set the user prefix, which is the root of all accessible user paths.
    /// The path has to exist and each of the paths accessible from the user_*
    /// methods of this class have to exist.
    /// \param[in] path An existing path with user write privileges.
    bool set_user_prefix(std::string const& path);

    /// Access the python context which allows execution of python scripts.
    /// \see PythonContext.
    /// \return python_context_.
    class Python {
    public:
        Python& load(std::string const& scriptname);

        template <typename... Args>
        Python& call(std::string const& function, Args const&... args) {

            result_.clear();
            build_format(args...);

            (void)tv::Environment::python_context_.execute_function(
                script_, function, result_, format_string_, args...);

            format_string_.clear();
            return *this;
        }

        Python& call(std::string const& function);

        std::string result(void);

    private:
        std::string script_;
        std::string result_;
        std::string format_string_;

        template <typename... Args>
        void build_format(Args const&... args) {
            format_string_.push_back('(');
            add_to_format(args...);
            format_string_.push_back(')');
        }

        template <typename... Args>
        void add_to_format(char const* s, Args const&... args) {
            format_string_.push_back('s');
            add_to_format(args...);
        }

        template <typename... Args>
        void add_to_format(int const& i, Args const&... args) {
            format_string_.push_back('i');
            add_to_format(args...);
        }

        // anchor
        void add_to_format(void) {}
    };

private:
    friend class Api;

    Environment(void) noexcept(noexcept(std::string()) and
                               noexcept(PythonContext()));

    PythonContext static python_context_;

    std::string static const system_modules_path_;
    std::string static const modules_dir_;
    std::string static const frames_dir_;
    std::string static const scripts_dir_;

    std::string user_prefix_;
    std::string user_modules_path_;
    std::string user_frames_path_;
    std::string user_scripts_path_;
};
}
