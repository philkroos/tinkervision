/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014-2015 philipp.kroos@fh-bielefeld.de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/** \file logger.hh
    \brief Debugging tool
*/

#ifndef LOGGER_H
#define LOGGER_H

#ifndef DEBUG
namespace tfv {

template <typename... Args>
void Log(std::string const& prefix, Args... args) {}

#else
#include <fstream>
#include <iostream>

namespace tfv {
class Logger {
private:
    std::string logfilename_ = "/tmp/tfv.log";
    std::ofstream logfile_;

    Logger(void) {
        logfile_.open(logfilename_, std::ios::out | std::ios::trunc);
    }

    ~Logger(void) { logfile_.close(); }

    template <typename T, typename... Args>
    void _print_out(T const& value, Args... args) {
        logfile_ << value;
        _print_out(args...);
    }

    template <typename T>
    void _print_out(T const& value) {
        logfile_ << value << std::endl;
    }

public:
    template <typename... Args>
    void log(std::string prefix, Args... args) {
        if (not logfile_.is_open()) {
            return;
        }

        logfile_ << prefix << ": ";
        _print_out(args...);
    }

    static Logger& instance(void) {
        static Logger logger;
        return logger;
    }
};

template <typename... Args>
void Log(std::string const& prefix, Args... args) {
    std::cout << "Logging for " << prefix << std::endl;
    Logger::instance().log(prefix, args...);
}

#endif
}
#endif
