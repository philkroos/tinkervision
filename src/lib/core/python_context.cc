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

bool tv::PythonContext::PythonScript::load(void) {
    if (module_ != nullptr) {  // already loaded
        return true;
    }
    auto module_name = PyString_FromString(script_.c_str());

    if (module_name == nullptr) {
        LogError("PYTHON_CONTEXT", "For script ", script_);
        return false;
    }

    module_ = PyImport_Import(module_name);
    Py_DECREF(module_name);

    if (module_ == nullptr) {
        LogError("PYTHON_CONTEXT", "For module ", script_);
        return false;
    }
    return true;
}

tv::PythonContext::~PythonContext(void) { Py_Finalize(); }

bool tv::PythonContext::is_valid_context(void) { return initialized_; }

bool tv::PythonContext::set_path(std::string const& pythonpath) noexcept {
    try {
        if (not is_directory(pythonpath)) {
            LogError("PYTHON_CONTEXT", "Not a valid path: ", pythonpath);
            return false;
        }

        // no-op if run twice
        Py_Initialize();

        auto addpath =
            std::string("import sys; sys.path.append('") + pythonpath + "');";
        if (0 != PyRun_SimpleString(addpath.c_str())) {
            LogError("PYTHON_CONTEXT", "PyRun error setting path ", pythonpath);
            return false;
        }

        module_path_ = pythonpath;

        // Needing this path during execute_script()
        if (module_path_.back() != '/') {
            module_path_.push_back('/');
        }

        Log("PYTHON_CONTEXT", "Set path: ", pythonpath);
        initialized_ = true;
        return true;

    } catch (...) {
        LogError("PYTHON_CONTEXT", "Error setting path ", pythonpath);
    }

    return false;
}

tv::PythonContext::PythonScript* tv::PythonContext::ScriptMap::get_script(
    std::string const& script) {

    auto ext = std::string();
    auto pyscript = strip_extension(script, ext);

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
