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

#ifndef DUMMY_H
#define DUMMY_H

#include "module.hh"

namespace tfv {
class Dummy : public Module {
public:
    Dummy(TFV_Int id, Module::Tag tags) : Module(id, "Dummy", tags) {}
    void execute(tfv::Image const& image) override final {}

    /**
     * \return \code ColorSpace::None to indicate that \code execute does not
     * have to be called.
     */
    ColorSpace expected_format(void) const override final {
        return ColorSpace::NONE;
    }
};

DECLARE_EMPTY_INTERFACE(Dummy)
}
#endif /* DUMMY_H */
