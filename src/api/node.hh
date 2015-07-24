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

#include <cassert>

#include "module.hh"
#include "image.hh"

namespace tfv {
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
     * \code module will only be executed if the provided image
     * is different from \code current_image, which is decided
     * by comparing both images timestamps for equality.
     * \param[in] image The image to be processed.
     */
    void execute(Image const& image);
    void execute_for_scene(Image const& image, TFV_Scene scene_id);

    TFV_Int module_id(void) const { return module_id_; }

    void add_scene(TFV_Scene scene_id) { scenes_.push_back(scene_id); }

    Node* parent(void) const { return parent_; }

    std::vector<Node*> children(void) const { return children_; }

    Node* get_child_from_module_id(TFV_Int module_id) {
        auto node = std::find_if(
            children_.begin(), children_.end(),
            [](Node const* node) { node->module_id() == module_id; });

        if (node == children_.end()) {
            return nullptr;
        }

        return &(*node);
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
               std::cend(scenes_);
    }

private:
    Image const* current_image = nullptr;
    TFV_Int module_id_;

    // Tree
    Node* parent_ = nullptr;
    std::vector<Node*> children_;

    // This node is part of these scenes
    std::vector<TFV_Scene> scenes_;
};
}