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
#include "logger.hh"

void tfv::Node::execute(ModuleExecutor executor, tfv::Timestamp timestamp) {

    Log("NODE::Execute", "(", (void*)this, ", module ", module_id_, ") at ",
        timestamp);

    if (timestamp_ != timestamp) {
        timestamp_ = timestamp;

        executor(module_id_);
    }

    // depth-first recursion
    for (auto node : children_) {
        node->execute(executor, timestamp);
    }
}

void tfv::Node::execute_for_scene(ModuleExecutor executor,
                                  tfv::Timestamp timestamp,
                                  TFV_Scene scene_id) {
    if (timestamp_ != timestamp) {
        timestamp_ = timestamp;

        executor(module_id_);
    }

    for (auto node : children_) {
        if (node->is_used_by_scene(scene_id)) {
            node->execute_for_scene(executor, timestamp, scene_id);
        }
    }
}
