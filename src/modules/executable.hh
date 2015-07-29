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

#ifndef EXECUTABLE_H
#define EXECUTABLE_H

#include "module.hh"

namespace tfv {

class Analysis : public Module {
protected:
    Analysis(TFV_Int id, std::string type, Module::Tag tags)
        : Module(id, type, tags |= Module::Tag::Analysis) {}

public:
    virtual void callback(void) const = 0;
    virtual void apply(tfv::Image& image) const = 0;
};

class Fx : public Module {
protected:
    Fx(TFV_Int id, std::string type, Module::Tag tags)
        : Module(id, type, tags |= Module::Tag::Fx) {}

public:
    virtual void apply(tfv::Image& image) const = 0;
};

class Output : public Module {
protected:
    Output(TFV_Int id, std::string type, Module::Tag tags)
        : Module(id, type, tags |= Module::Tag::Output) {}
};
}
#endif /* EXECUTABLE_H */
