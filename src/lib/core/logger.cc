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

#if defined(WITH_LOGGER)

#include "logger.hh"
#include "module.hh"
#include "tv_module.hh"
#include "scenetrees.hh"

std::string tfv::Logger::PREFIX_WARNING = "WARNING";
std::string tfv::Logger::PREFIX_ERROR = "ERROR";

std::ostream& tfv::operator<<(std::ostream& os, tfv::Module* module) {

    auto tags_bits = std::bitset<16>(
        static_cast<std::underlying_type<Module::Tag>::type>(module->tags()));

    os << "Id: " << module->id() << " Tags: " << tags_bits;

    return os;
}

std::ostream& tfv::operator<<(std::ostream& os, tfv::SceneTree const& tree) {

    std::function<void(Node const&, size_t)> node_recursion =
        [&](Node const& node, size_t siblings) {

        os << node.module_id();
        if (not node.is_leaf()) {
            auto children = node.children().size();
            os << " (";
            for (auto child : node.children()) {
                node_recursion(*child, --children);
            }
            os << ")";
        }
        if (siblings > 0) {
            os << " ";
        }
    };

    auto node = tree.root();

    os << "(";
    node_recursion(node, 0);
    os << ")" << std::endl;

    return os;
}

std::ostream& tfv::operator<<(std::ostream& os, tfv::ColorSpace const& format) {
    switch (format) {
        case tfv::ColorSpace::INVALID:
            os << "INVALID";
            break;
        case tfv::ColorSpace::YUYV:
            os << "YUYV";
            break;
        case tfv::ColorSpace::YV12:
            os << "YV12";
            break;
        case tfv::ColorSpace::BGR888:
            os << "BGR";
            break;
        case tfv::ColorSpace::RGB888:
            os << "RGB";
            break;
        default:
            os << "??UNKNOWN??";
            break;
    }
    return os;
}

std::ostream& tfv::operator<<(std::ostream& os,
                              tfv::ImageHeader const& header) {
    // Header:WxH,Bytesize,Format
    os << header.width << "x" << header.height << "," << header.bytesize << ","
       << header.format;
    return os;
}

std::ostream& tfv::operator<<(std::ostream& os, tfv::Timestamp ts) {
    os << ts.time_since_epoch().count();

    return os;
}

std::ostream& tfv::operator<<(std::ostream& os, TFV_Id id) {
    os << static_cast<int>(id);
    return os;
}
#endif
