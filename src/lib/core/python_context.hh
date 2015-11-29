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

#ifndef PYTHON_CONTEXT_H
#define PYTHON_CONTEXT_H

#include <Python.h>
#include <structmember.h>
#include <string>
#include <unordered_map>

#include "filesystem.hh"
#include "logger.hh"

namespace tv {

class PythonContext {

    class PythonScript {
    private:
        std::string script_;
        PyObject* module_{nullptr};

    public:
        PythonScript(void) = default;
        PythonScript(std::string const& script) : script_(script) {}
        ~PythonScript(void) {
            if (module_) {
                Py_DECREF(module_);
            }
        }

        bool load(void);

        template <typename... Args>
        bool call(std::string const& function, std::string& result,
                  std::string& format, Args const&... args) {

            auto fun = PyObject_GetAttrString(module_, function.c_str());

            if (fun == nullptr) {
                LogError("PYTHON_CONTEXT", "For function from ", script_);
                return false;
            }

            auto arguments = Py_BuildValue(format.c_str(), args...);
            if (arguments == nullptr) {
                LogError("PYTHON_CONTEXT", "For arguments ", args...);
                return false;
            }

            auto res = PyObject_CallObject(fun, arguments);

            Py_DECREF(fun);
            Py_DECREF(arguments);
            if (res == nullptr) {
                LogError("PYTHON_CONTEXT", "For result from ", script_);
                return false;
            }

            if (PyString_Check(res)) {
                result = std::string(PyString_AsString(res));
            } else if (PyInt_Check(res)) {
                result = std::to_string(PyInt_AsLong(res));
            } else {
                LogWarning("PYTHON_CONTEXT", "Invalid result from ", script_,
                           ": ", res->ob_type->tp_name);
            }

            Log("PYTHON_CONTEXT", "Result from ", script_, ": ", result);

            Py_DECREF(res);

            return true;
        }
    };

    class ScriptMap {
    public:
        /// Normalizes passed string and retrieves script.
        PythonScript* get_script(std::string const& script);

    private:
        using Scripts = std::unordered_map<std::string, PythonScript>;

        Scripts scripts_;
    };

    ScriptMap scripts_;

public:
    ~PythonContext(void);

    /// One-time-op: Set the path where python modules are loaded from.
    /// This also calls Py_Initialize.
    /// \note Will only accept a valid path once.
    /// \param[in] pythonpath Will be added to sys.path.
    /// \return true if the path is a directory in the filesystem.
    bool set_path(std::string const& pythonpath) noexcept;

    /// Execute a function from a script in the path set with set_path().
    /// \param[in] script A script in module_path_.
    /// \param[in] function A function defined in script
    /// \param[out] result Result of calling the function, if any.
    /// \param[in] format Format string of the parameters.
    /// \return true If the module exists in module_path_.
    template <typename... Args>
    bool execute_function(std::string const& script,
                          std::string const& function, std::string& result,
                          std::string& format, Args const&... args) {

        if (not is_valid_context()) {
            return false;
        }

        auto pyscript = scripts_.get_script(script);
        if (not pyscript->load()) {
            return false;
        }

        return pyscript->call(function, result, format, args...);
    }

private:
    std::string module_path_;
    bool initialized_{false};  ///< True after successfull set_path().

    bool is_valid_context(void);
    bool is_script_loaded(void);
};
}

#endif
