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

#include <Python.h>

#include "python_context.hh"

#include "filesystem.hh"
#include "logger.hh"

tv::PythonContext::~PythonContext(void) { Py_Finalize(); }

bool tv::PythonContext::execute_script(std::string const& script,
                                       std::string& result) {
    if (not is_valid_context()) {
        return false;
    }

    auto ext = std::string();
    auto pyscript = strip_extension(script, ext);

    if (not ext.empty() and ext != "py") {
        return false;
    }

    Log("PYTHON_CONTEXT", "Executing ", pyscript);

    auto module_name = PyString_FromString(script.c_str());

    if (module_name == nullptr) {
        LogError("PYTHON_CONTEXT", "For script ", script);
        return false;
    }

    auto module = PyImport_Import(module_name);
    Py_DECREF(module_name);

    if (module == nullptr) {
        LogError("PYTHON_CONTEXT", "For module ", script);
        return false;
    }

    auto fun1 = PyObject_GetAttrString(module, "tv_external_set");
    auto fun2 = PyObject_GetAttrString(module, "tv_external_get");
    Py_DECREF(module);

    if (fun1 == nullptr or fun2 == nullptr) {
        LogError("PYTHON_CONTEXT", "For function from ", script);
        return false;
    }

    // args not yet supported
    // auto args = Py_BuildValue("(s)", arg.c_str());
    // auto result = PyObject_CallObject(fun, args);

    (void)PyObject_CallObject(fun1, nullptr);
    auto res = PyObject_CallObject(fun2, nullptr);
    Py_DECREF(fun1);
    Py_DECREF(fun2);
    // Py_DECREF(args);

    if (res == nullptr) {
        LogError("PYTHON_CONTEXT", "For result from ", script);
        return false;
    }

    // Convert the result to a std::string.
    result = std::string(PyString_AsString(res));
    Py_DECREF(res);

    Log("PYTHON_CONTEXT", "Result from ", script, ": ", result);

    return true;
}

bool tv::PythonContext::is_valid_context(void) { return initialized_; }

bool tv::PythonContext::set_path(std::string const& pythonpath) noexcept {
    if (initialized_) {
        return false;
    }

    try {
        if (not is_directory(pythonpath)) {
            LogError("PYTHON_CONTEXT", "Not a valid path: ", pythonpath);
            initialized_ = false;
            return false;
        }

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
