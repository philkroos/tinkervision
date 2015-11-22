/// \file python_context.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declares the class PythonContext.
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

namespace tv {
class PythonContext {
public:
    ~PythonContext(void);

    /// One-time-op: Set the path where python modules are loaded from.
    /// This also calls Py_Initialize.
    /// \note Will only accept a valid path once.
    /// \param[in] pythonpath Will be added to sys.path.
    /// \return true if the path is a directory in the filesystem.
    bool set_path(std::string const& pythonpath) noexcept;

    /// Execute a script from the path set with set_path().
    /// \param[in] A script in module_path_.
    /// \return true If the module exists in module_path_.
    bool execute_script(std::string const& script, std::string& result);

private:
    std::string module_path_;
    bool initialized_{false};  ///< True after successfull set_path().

    bool is_valid_context(void);
};
}
