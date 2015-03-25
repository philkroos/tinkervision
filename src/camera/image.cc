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

#include "image.hh"

std::ostream& tfv::operator<<(std::ostream& ost,
                              tfv::ImageFormat const& format) {
    switch (format) {
        case tfv::ImageFormat::INVALID:
            ost << "INVALID";
            break;
        case tfv::ImageFormat::YUYV:
            ost << "YUYV";
            break;
        case tfv::ImageFormat::YV12:
            ost << "YV12";
            break;
        case tfv::ImageFormat::BGR888:
            ost << "BGR";
            break;
        case tfv::ImageFormat::RGB888:
            ost << "RGB";
            break;
        default:
            ost << "??UNKNOWN??";
            break;
    }
    return ost;
}
