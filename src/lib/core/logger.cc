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

#if defined(DEBUG) || defined(WITH_LOGGER)

#include "logger.hh"
#include "module.hh"
#include "scenetrees.hh"

std::string tfv::Logger::PREFIX_WARNING = "WARNING";
std::string tfv::Logger::PREFIX_ERROR = "ERROR";

std::ostream& tfv::operator<<(std::ostream& stream, tfv::Module* module) {

    auto tags_bits = std::bitset<16>(
        static_cast<std::underlying_type<Module::Tag>::type>(module->tags()));

    stream << "Id: " << module->id() << " Tags: " << tags_bits;

    return stream;
}

std::ostream& tfv::operator<<(std::ostream& stream,
                              tfv::SceneTree const& tree) {

    std::function<void(Node const&, size_t)> node_recursion =
        [&](Node const& node, size_t siblings) {

        stream << node.module_id();
        if (not node.is_leaf()) {
            auto children = node.children().size();
            stream << " (";
            for (auto child : node.children()) {
                node_recursion(*child, --children);
            }
            stream << ")";
        }
        if (siblings > 0) {
            stream << " ";
        }
    };

    auto node = tree.root();

    stream << "(";
    node_recursion(node, 0);
    stream << ")" << std::endl;

    return stream;
}

std::ostream& tfv::operator<<(std::ostream& ost,
                              tfv::ColorSpace const& format) {
    switch (format) {
        case tfv::ColorSpace::INVALID:
            ost << "INVALID";
            break;
        case tfv::ColorSpace::YUYV:
            ost << "YUYV";
            break;
        case tfv::ColorSpace::YV12:
            ost << "YV12";
            break;
        case tfv::ColorSpace::BGR888:
            ost << "BGR";
            break;
        case tfv::ColorSpace::RGB888:
            ost << "RGB";
            break;
        default:
            ost << "??UNKNOWN??";
            break;
    }
    return ost;
}

std::ostream& tfv::operator<<(std::ostream& stream, tfv::Timestamp ts) {
    stream << ts.time_since_epoch().count();

    return stream;
}

std::ostream& tfv::operator<<(std::ostream& stream, TFV_Id id) {
    stream << static_cast<int>(id);
    return stream;
}

#endif
