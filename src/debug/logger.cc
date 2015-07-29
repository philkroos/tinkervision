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

#ifdef DEBUG

#include "logger.hh"
#include "module.hh"

std::string tfv::Logger::PREFIX_WARNING = "WARNING";
std::string tfv::Logger::PREFIX_ERROR = "ERROR";

std::ostream& tfv::operator<<(std::ostream& stream, tfv::Module* module) {
    auto tags_bits = std::bitset<16>(
        static_cast<std::underlying_type<Module::Tag>::type>(module->tags()));

    stream << "Id: " << module->id() << " Tags: " << tags_bits;
    return stream;
}

#endif
