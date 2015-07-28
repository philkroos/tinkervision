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

/** \file node.cc
*/

#include "node.hh"

void tfv::Node::execute(Modules& modules, tfv::Image const& image) {
    if (not current_image or (current_image->timestamp != image.timestamp)) {
        current_image = &image;

        if (modules.managed(module_id_)) {
            // module.execute(*current_image);
        }
    }

    for (auto node : children_) {
        node->execute(modules, image);
    }
}

void tfv::Node::execute_for_scene(Modules& modules, tfv::Image const& image,
                                  TFV_Scene scene_id) {
    if (not current_image or (current_image->timestamp != image.timestamp)) {
        current_image = &image;

        if (modules.managed(module_id_)) {
            // modules.execute(*current_image);
        }
    }

    for (auto node : children_) {
        if (node->is_used_by_scene(scene_id)) {
            node->execute_for_scene(modules, image, scene_id);
        }
    }
}
