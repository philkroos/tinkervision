/// \file python_context.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Defines the class PythonContext.
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

#include "python_context.hh"

#include <algorithm>

static bool is_dir(std::string const& fullname) {
    struct stat buffer;
    return (stat(fullname.c_str(), &buffer) == 0) and S_ISDIR(buffer.st_mode);
}

static std::string strip(std::string const& filename, std::string& extension) {
    auto rev_it = std::find(filename.rbegin(), filename.rend(), '.');

    if (rev_it == filename.rend() or
        rev_it == filename.rend() - 1) {  // no extension or dotfile
        extension = "";
        return filename;
    }

    extension = std::string(rev_it.base(), filename.cend());
    return std::string(filename.cbegin(), rev_it.base() - 1);
}

bool tv::PythonContext::PythonScript::load(void) {
    if (module_ != nullptr) {  // already loaded
        return true;
    }
    auto module_name = PyString_FromString(script_.c_str());

    if (module_name == nullptr) {
        return false;
    }

    module_ = PyImport_Import(module_name);
    Py_DECREF(module_name);

    if (module_ == nullptr) {
        return false;
    }
    return true;
}

tv::PythonContext::~PythonContext(void) { Py_Finalize(); }

bool tv::PythonContext::is_valid_context(void) { return initialized_; }

bool tv::PythonContext::set_path(std::string const& pythonpath) noexcept {
    try {
        if (not is_dir(pythonpath)) {
            return false;
        }

        // no-op if run twice
        Py_Initialize();

        auto addpath =
            std::string("import sys; sys.path.append('") + pythonpath + "');";
        if (0 != PyRun_SimpleString(addpath.c_str())) {
            return false;
        }

        module_path_ = pythonpath;

        // Needing this path during execute_script()
        if (module_path_.back() != '/') {
            module_path_.push_back('/');
        }

        initialized_ = true;
        return true;

    } catch (...) {
    }

    return false;
}

tv::PythonContext::PythonScript* tv::PythonContext::ScriptMap::get_script(
    std::string const& script) {

    auto ext = std::string();
    auto pyscript = strip(script, ext);

    /// Script name can be passed without extension, in which case
    /// .py is assumed, or with extension .py.
    if (not ext.empty() and ext != "py") {
        return nullptr;
    }

    if (not scripts_.count(pyscript)) {
        scripts_[pyscript] = PythonScript(pyscript);
    }

    return &scripts_[pyscript];
}
