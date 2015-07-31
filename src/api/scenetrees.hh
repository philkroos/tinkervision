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

/**
 * \file scenetrees.hh
 * \brief Declares the modules SceneTrees and SceneTree which manage scenes.
 *
 * A tree is merely a view on the scenes managed. A scene itself
 * provides a manner of ordered module execution. It is implemented as
 * a linked list of instances of \code Node, which are wrappers around
 * the actual modules. However, multiple scenes may share the same
 * nodes. As long as the first nodes of two scenes are equal, it makes
 * sense to share the nodes between them, preventing multiple
 * executions, resource conflicts and generally have a reasonable
 * internal representation of the scenes setup.  Effectively, this
 * idea leans toward the representation of scenes merged into
 * trees. This is what this module does. A \code SceneTree hereby is
 * the view on related scenes (i.e. linked nodes with equal
 * beginning).  \code SceneTrees wraps all trees into a single
 * interface and also holds a synchronized container (\code
 * SharedResource) of all created nodes, including the logic of
 * creating or destroying individual nodes and linking them into the
 * correct tree.
 *
 * \see Node.hh
 *
 * \see SharedResource.hh
 */

#ifndef SCENETREES_H
#define SCENETREES_H

#include "tinkervision_defines.h"
#include "shared_resource.hh"
#include "node.hh"
#include "scene.hh"

namespace tfv {
class SceneTree {
private:
    Node* root_;  ///< Rootnode may refer to multiple scenes.
    std::vector<TFV_Scene> scenes_;

    bool active_ = false;

public:
    explicit SceneTree(Node* root) : root_(root) {
        assert(root->scenes()->size() == 1);
        scenes_.push_back(root->scenes()->at(0));
        root->set_tree(this);
    }

    /**
     * Retrieve the root of this tree.
     * \return reference to the root node.
     */
    Node& root(void) const { return *root_; }

    /**
     * Check if this tree contains the scene \code id.
     * \param[in] id The scene searched for.
     * \return \code t if the \code root_ is associated with the scene \code id.
     */
    bool contains_scene(TFV_Scene id) const {
        return std::find(scenes_.cbegin(), scenes_.cend(), id) !=
               scenes_.cend();
    }

    bool active(void) const { return active_; }
    void activate(void) { active_ = true; }
    void deactivate(void) { active_ = false; }
};

class SceneTrees {
private:
    using Nodes = SharedResource<Node>;

    Nodes scene_nodes_;
    std::vector<SceneTree> scene_trees_;

public:
    /**
     * Are there active nodes/scene trees?
     * \return \code empty_
     */
    bool empty(void) const { return scene_trees_.size() == 0; }

    /**
     * \note \code scene_id must be unique, it is not checked here but
     * things will break badly if it's not. Also, it is not checked
     * here if the module for \code module_id does exist.
     */
    TFV_Result scene_start(TFV_Scene scene_id, TFV_Int module_id);

    TFV_Result add_to_scene(TFV_Scene scene_id, TFV_Int module_id);

    void exec_all(void);
    void exec_scene(TFV_Scene scene_id);

private:
    TFV_Int _next_node_id(void) const {
        static TFV_Int node_id{std::numeric_limits<TFV_Id>::max() + 1};
        return node_id++;
    }
};
}

#endif /* SCENETREES_H */
