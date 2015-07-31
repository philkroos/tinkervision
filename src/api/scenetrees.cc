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

#include <thread>
#include <chrono>

#include "scenetrees.hh"

TFV_Result tfv::SceneTrees::scene_start(TFV_Scene scene_id, TFV_Int module_id) {

    // same root existing already?
    auto tree = std::find_if(scene_trees_.begin(), scene_trees_.end(),
                             [&module_id](SceneTree const& tree) {
        return tree.root().module_id() == module_id;
    });

    if (tree == scene_trees_.end()) {
        // Allocate a new node as root of a new tree. The tree is not
        // active yet since the allocated node has not been persisted
        // (See exec*). The reference to the tree in Node is being set
        // in Tree's constructor.
        auto const node_id = _next_node_id();

        if (not scene_nodes_.allocate<Node>(
                node_id, [&](Node& node) { scene_trees_.emplace_back(&node); },
                scene_id, module_id)) {
            return TFV_NODE_ALLOCATION_FAILED;
        }
        return TFV_OK;

    } else {  // can reuse an existing root
        auto const node_id = tree->root().id();
        return scene_nodes_.exec_one(node_id, [&](Node& node) {
            std::cout << "Adding " << scene_id << std::endl;
            node.add_to_scene(scene_id);
            return TFV_OK;
        });
    }
}

TFV_Result tfv::SceneTrees::add_to_scene(TFV_Scene scene_id,
                                         TFV_Int module_id) {
    Log("SCENETREE::AddToScene", module_id, " -> ", scene_id);

    auto leaf_of_scene = scene_nodes_.find_if([&scene_id](Node const& node) {
        // FIXME: This is not correct. If used by several scenes,
        // not searching for a leaf here. Have to mark each scenes
        // leaf somehow.
        return node.is_leaf() and node.is_used_by_scene(scene_id);
    });

    if (nullptr == leaf_of_scene) {  // no such scene (or a bug:))
        std::cout << "Invalid scene" << std::endl;
        return TFV_INVALID_ID;
    }

    auto leaf_id = leaf_of_scene->id();

    // Check for existing node in a synchronized and locked context
    auto result = scene_nodes_.exec_one(leaf_id, [&](Node& node) {

        assert(nullptr != scene_nodes_.access_unlocked(leaf_id));

        auto req_node = node.get_child_from_module_id(module_id);
        if (req_node != nullptr) {
            // Another scene owns the same nodes, and the requested node
            // also already exists in that scene. It can be reused.
            req_node->add_to_scene(scene_id);
            return TFV_OK;
        }

        return TFV_INTERNAL_NODE_UNCONFIGURED;
    });

    if (result != TFV_INTERNAL_NODE_UNCONFIGURED) {
        return result;
    }

    // Need to setup a new node as child of the scenes leaf.
    // The reference to the tree is being set in Node's constructor.
    auto node_id = _next_node_id();
    if (not scene_nodes_.allocate<Node>(node_id, nullptr, scene_id, module_id,
                                        leaf_of_scene)) {
        return TFV_NODE_ALLOCATION_FAILED;
    }
    return TFV_OK;
}

void tfv::SceneTrees::exec_all(Node::ModuleExecutor executor,
                               Timestamp timestamp) {
    // persist and link each allocated node into its tree
    scene_nodes_.update([&](Node& new_node) {
        auto parent = new_node.parent();
        if (nullptr == parent) {  // it was a root, a new tree.
            new_node.tree()->activate();
        } else {
            parent->add_child(&new_node);
        }
    });

    for (auto& tree : scene_trees_) {
        Log("SCENETREES::exec_all", "Moduletree: ", tree);
        tree.execute(executor, timestamp);
    }
}

void tfv::SceneTrees::exec_scene(TFV_Scene scene_id) {}
