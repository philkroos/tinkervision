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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>

namespace tfv {

class Exception : public std::exception {
private:
    std::string message_;

protected:
    Exception(std::string classname, std::string error)
        : message_{"tfv::Exception in class " + classname + ": " + error} {}

public:
    virtual ~Exception(void) = default;
    virtual const char* what(void) const noexcept { return message_.c_str(); }
};

struct ConstructionException : public Exception {
public:
    ConstructionException(std::string classname, std::string error)
        : Exception(classname, "During construction: " + error) {}

    ~ConstructionException(void) override = default;
};
}
#endif /* EXCEPTIONS_H */
