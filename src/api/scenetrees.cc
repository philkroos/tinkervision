/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2015 philipp.kroos@fh-bielefeld.de

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
    auto it = std::find_if(scene_trees_.begin(), scene_trees_.end(),
                           [&module_id](SceneTree* tree) {
        return tree->root().module_id() == module_id;
    });

    if (it == scene_trees_.end()) {
        // Allocate a new node as root of a new tree. The tree is not
        // active yet since the allocated node has not been persisted
        // (See exec*). The reference to the tree in Node is set
        // in Tree's constructor.
        auto const node_id = _next_node_id();

        Log("SCENETREES::scene_start", "Allocating new tree");
        if (not scene_nodes_.allocate<Node>(
                node_id, [&](Node& node) {
                             scene_trees_.emplace_back(new SceneTree(&node));
                             node.tree()->activate();  // was in update
                             node.tree()->add_node_to_scene(
                                 scene_id, &node);  // was in update
                         },
                scene_id, module_id)) {
            return TFV_NODE_ALLOCATION_FAILED;
        }
        return TFV_OK;

    } else {  // can reuse an existing root
        auto tree = *it;
        auto const node_id = tree->root().id();
        return scene_nodes_.exec_one(node_id, [&](Node& node) {
            std::cout << "Adding " << scene_id << std::endl;
            node.add_to_scene(scene_id);
            tree->add_node_to_scene(scene_id, &node);
            tree->log_scenes();
            return TFV_OK;
        });
    }
}

TFV_Result tfv::SceneTrees::add_to_scene(TFV_Scene scene_id,
                                         TFV_Int module_id) {
    Log("SCENETREES::AddToScene", module_id, " -> ", scene_id);

    auto it = std::find_if(
        scene_trees_.begin(), scene_trees_.end(),
        [&](SceneTree* tree) { return tree->contains_scene(scene_id); });

    if (scene_trees_.end() == it) {  // no such scene (or a bug:))
        std::cout << "Invalid scene" << std::endl;
        return TFV_INVALID_ID;
    }

    auto tree = *it;
    // get leaf of scene, not necessarily tree
    auto leaf_of_scene = tree->leaf_of_scene(scene_id);
    auto leaf_id = leaf_of_scene->id();
    Log("SCENETREES::AddToScene", "Node ", leaf_id, "/",
        leaf_of_scene->module_id(), " selected");

    // Check if module_id exists as child of leaf_id in a synchronized
    // and locked context
    auto result = scene_nodes_.exec_one(leaf_id, [&](Node& node) {

        assert(nullptr != scene_nodes_.access_unlocked(leaf_id));

        auto req_node = node.get_child_from_module_id(module_id);
        if (req_node != nullptr) {
            // Another scene owns the same nodes, and the requested node
            // also already exists in that scene. It can be reused.
            Log("SCENETREES::AddToScene", "Reusing ", req_node->id(), "/",
                req_node->module_id());
            req_node->add_to_scene(scene_id);
            tree->add_node_to_scene(scene_id, req_node);
            tree->log_scenes();
            return TFV_OK;
        }

        return TFV_INTERNAL_NODE_UNCONFIGURED;
    });

    // No new node created, other scene reused
    if (result != TFV_INTERNAL_NODE_UNCONFIGURED) {
        return result;
    }

    // Need to setup a new node as child of the scenes leaf.
    // The reference to the tree is set in Node's constructor.
    auto node_id = _next_node_id();
    Log("SCENETREES::AddToScene", "Creating new node ", node_id);
    tree->log_scenes();  // ok
    if (not scene_nodes_.allocate<Node>(
            node_id, [&](Node& node) {
                         // node.set_tree(leaf_of_scene->tree()); // in node
                         leaf_of_scene->add_child(&node);  // in update
                         node.tree()->add_node_to_scene(
                             scene_id, &node);  // was in update
                     },
            scene_id, module_id, leaf_of_scene)) {
        return TFV_NODE_ALLOCATION_FAILED;
    }

    return TFV_OK;
}

void tfv::SceneTrees::exec_all(Node::ModuleExecutor executor,
                               Timestamp timestamp) {
    /*
    // persist and link each allocated node into its tree
    scene_nodes_.update([&](Node& new_node) {
        new_node.tree()->add_node_to_scene(new_node.scenes()->at(0), &new_node);

        auto parent = new_node.parent();
        if (nullptr == parent) {  // it was a root, a new tree.
            new_node.tree()->activate();
        } else {
            parent->add_child(&new_node);
        }

        assert(not parent or
               parent->get_child_from_module_id(new_node.module_id()) ==
                   &new_node);
    });
    */

    for (auto& tree : scene_trees_) {
        tree->log_scenes();
        Log("SCENETREES::exec_all", "Moduletree: ", *tree);
        tree->execute(executor, timestamp);
    }
}

void tfv::SceneTrees::exec_scene(TFV_Scene scene_id) {}
