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

/** \file node.hh
*/

#include "module.hh"
#include "image.hh"

namespace tfv {
    class Node {
    public:
        explicit Node(Module& module) :
            module(module) {}

        /**
         * Execute the module held by this node.
         *
         * \code module will only be executed if the provided image
         * is different from \code current_image, which is decided
         * by comparing both images timestamps for equality.
         * \param[in] image The image to be processed.
         */
        void execute(Image const& image);
    private:
        Image const* current_image = nullptr;
        Module& module;
    };
}
