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

#ifndef NODE_H
#define NODE_H

#include <cassert>
#include <vector>
#include <algorithm>

#include "module.hh"
#include "image.hh"
#include "shared_resource.hh"
#include "module.hh"

namespace tfv {
using Modules = tfv::SharedResource<tfv::Module>;

class Node {

public:
    // default c'tor to be able to store in container types
    Node(void) : module_id_(TFV_UNUSED_ID) {}

    // c'tor for a root node
    Node(TFV_Scene scene_id, TFV_Int module_id)
        : Node(scene_id, module_id, nullptr) {}

    // complete c'tor
    Node(TFV_Scene scene_id, TFV_Int module_id, Node* parent)
        : module_id_(module_id), parent_(parent) {

        scenes_.push_back(scene_id);
        if (parent) {
            parent_->children_.push_back(this);
        }
    }

    /**
     * Execute the module held by this node.
     *
     * \code module will only be executed if the provided timestamp
     * is different from \code timestamp_.
     * \param[in] executor The function to be called on the module.
     * \param[in] timestamp The image to be processed.
     */
    void execute(std::function<void(TFV_Int id)> executor, Timestamp timestamp);
    void execute_for_scene(std::function<void(TFV_Int id)> executor,
                           Timestamp timestamp, TFV_Scene scene_id);

    TFV_Int module_id(void) const { return module_id_; }

    void add_scene(TFV_Scene scene_id) { scenes_.push_back(scene_id); }

    Node* parent(void) const { return parent_; }

    void set_parent(Node* parent) { parent_ = parent; }

    std::vector<TFV_Scene>* scenes(void) { return &scenes_; }

    std::vector<Node*> children(void) const { return children_; }

    Node* get_child_from_module_id(TFV_Int module_id) {
        auto node = std::find_if(children_.begin(), children_.end(),
                                 [&module_id](Node const* node) {
            return node->module_id() == module_id;
        });

        if (node == children_.end()) {
            return nullptr;
        }

        return *node;
    }

    bool is_leaf(void) const { return children_.empty(); }

    void remove_scene(TFV_Scene scene_id) {
        auto it = std::find(scenes_.begin(), scenes_.end(), scene_id);

        assert(it != scenes_.end());

        scenes_.erase(it);
    }

    bool is_used_by_any_scene(void) const { return not scenes_.empty(); }

    bool is_used_by_scene(TFV_Scene id) const {
        return std::find(scenes_.cbegin(), scenes_.cend(), id) !=
               scenes_.cend();
    }

    void add_child(Node* node) {
        children_.push_back(node);

        assert(node->parent() == this);
        // node->parent = this;
    }

    void remove_child(Node* node) {
        auto it = std::find(children_.begin(), children_.end(), node);
        if (it != children_.end()) {
            children_.erase(it);
        }
    }

private:
    Timestamp timestamp_{0};
    TFV_Int module_id_;

    // Tree
    Node* parent_ = nullptr;
    std::vector<Node*> children_;

    // This node is part of these scenes
    std::vector<TFV_Scene> scenes_;
};
}

#endif
