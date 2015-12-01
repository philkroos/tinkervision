/// \file logger.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Defines the Logger class of Tinkervision.
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

#if defined(WITH_LOGGER)

#include "logger.hh"
#include "module_wrapper.hh"
#include "module.hh"
#include "scenetrees.hh"

std::string tv::Logger::PREFIX_DEBUG = "D";
std::string tv::Logger::PREFIX_WARNING = "W";
std::string tv::Logger::PREFIX_ERROR = "E";

std::ostream& tv::operator<<(std::ostream& os, tv::ModuleWrapper* module) {

    auto tags_bits = std::bitset<16>(
        static_cast<std::underlying_type<ModuleWrapper::Tag>::type>(
            module->tags()));

    os << "Id: " << module->id() << " Tags: " << tags_bits;

    return os;
}

std::ostream& tv::operator<<(std::ostream& os, tv::SceneTree const& tree) {

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

std::ostream& tv::operator<<(std::ostream& os, tv::ColorSpace const& format) {
    switch (format) {
        case tv::ColorSpace::NONE:
            os << "NONE";
            break;
        case tv::ColorSpace::INVALID:
            os << "INVALID";
            break;
        case tv::ColorSpace::YUYV:
            os << "YUYV";
            break;
        case tv::ColorSpace::YV12:
            os << "YV12";
            break;
        case tv::ColorSpace::BGR888:
            os << "BGR";
            break;
        case tv::ColorSpace::RGB888:
            os << "RGB";
            break;
        default:
            os << "??UNKNOWN??";
            break;
    }
    return os;
}

std::ostream& tv::operator<<(std::ostream& os, tv::ImageHeader const& header) {
    // Header:WxH,Bytesize,Format
    os << header.width << "x" << header.height << "," << header.bytesize << ","
       << header.format;
    return os;
}

std::ostream& tv::operator<<(std::ostream& os, tv::Timestamp ts) {
    os << ts.time_since_epoch().count();

    return os;
}

std::ostream& tv::operator<<(std::ostream& os, int8_t id) {
    os << std::to_string(id);
    return os;
}

std::ostream& tv::operator<<(std::ostream& os, uint8_t id) {
    os << std::to_string(id);
    return os;
}

std::ostream& tv::operator<<(std::ostream& os, StringParameter& p) {
    std::string value;
    (void)p.get(value);
    os << p.name() << ": " << value;
    return os;
}

std::ostream& tv::operator<<(std::ostream& os, NumericalParameter& p) {
    int32_t value;
    (void)p.get(value);
    os << p.name() << ": " << value << " [" << p.min() << "," << p.max() << "]";
    return os;
}

#endif
