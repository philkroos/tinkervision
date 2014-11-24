/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014 philipp.kroos@fh-bielefeld.de

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

#ifndef FRAME_H
#define FRAME_H

#include <unordered_map>
#include <iostream>

#include "tinkervision_defines.h"

namespace tfv {

typedef struct Frame_ {
    TFV_Id id;
    int rows;
    int columns;
    TFV_ImageData* data;
    Frame_(TFV_Id id, int rows, int columns, int channels)
        : id(id),
          rows(rows),
          columns(columns),
          data(new TFV_ImageData[rows * columns * channels]) {
        std::cout << "Allocating " << id << ", " << (void*)this << std::endl;
        std::cout << "With rxc " << rows << "x" << columns << std::endl;
    }
    ~Frame_(void) {
        std::cout << "Deleting " << id << ", " << (void*)this << std::endl;
        std::cout << "With rxc " << rows << "x" << columns << std::endl;
        if (data) {
            delete[] data;
        }
    }
} Frame;
}
#endif /* FRAME_H */
