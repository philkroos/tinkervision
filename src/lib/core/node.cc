/// \file node.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Defines the class Node.
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

#include "node.hh"
#include "logger.hh"
#include "scenetrees.hh"

tv::Node::Node(int16_t node_id, int16_t scene_id, int16_t module_id,
               Node* parent) noexcept(noexcept(std::vector<Node*>()) and
                                      noexcept(std::vector<int16_t>()))
    : id_(node_id), module_id_(module_id) {

    scenes_.push_back(scene_id);
    Log("NODE::c'tor", (void*)this, " Scene: ", scene_id, " Parent: ",
        (void*)parent);

    set_parent(parent);
    Log("NODE::c'tor", "Done");
}

void tv::Node::execute(ModuleExecutor executor, tv::Timestamp timestamp) {

    Log("NODE::Execute", "(",
        (void*)this);  //, ", module ", module_id_, ") at ",
    //        timestamp);

    if (timestamp_ != timestamp) {
        timestamp_ = timestamp;

        executor(module_id_);
    }

    // depth-first recursion
    for (auto node : children_) {
        node->execute(executor, timestamp);
    }
}

void tv::Node::execute_for_scene(ModuleExecutor executor,
                                 tv::Timestamp timestamp, int16_t scene_id) {
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
