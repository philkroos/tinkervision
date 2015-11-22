/// \file scenetrees.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Declares the modules SceneTrees and SceneTree which manage scenes.
/// A tree is merely a view on the scenes managed. A scene itself
/// provides a manner of ordered module execution. It is implemented as
/// a linked list of instances of Node, which are wrappers around
/// the actual modules. However, multiple scenes may share the same
/// nodes. As long as the first nodes of two scenes are equal, it makes
/// sense to share the nodes between them, preventing multiple
/// executions, resource conflicts and generally have a reasonable
/// internal representation of the scenes setup.  Effectively, this
/// idea leans toward the representation of scenes merged into
/// trees. This is what this module does. A SceneTree hereby is
/// the view on related scenes (i.e. linked nodes with equal
/// beginning).  SceneTrees wraps all trees into a single
/// interface and also holds a synchronized container (SharedResource)
/// of all created nodes, including the logic of creating or destroying
/// individual nodes and linking them into the correct tree.
///
/// \see Node.hh
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

#ifndef SCENETREES_H
#define SCENETREES_H

#include <map>

#include "tinkervision_defines.h"
#include "shared_resource.hh"
#include "node.hh"
#include "scene.hh"
#include "logger.hh"

namespace tv {

/// A tree-like representation of linked Module's.
class SceneTree {
private:
    Node* root_;  ///< Rootnode may refer to multiple scenes.
    std::map<int16_t, Node*> mutable scenes_;
    std::mutex mutable exec_lock_;

    bool active_ = false;

public:
    explicit SceneTree(Node* root) : root_(root) {
        assert(root->scenes()->size() == 1);
        scenes_[root->scenes()->at(0)] = root;
        root->set_tree(this);
    }

    SceneTree(SceneTree const&) = delete;
    SceneTree& operator=(SceneTree const&) = delete;

    /// Retrieve the root of this tree.
    /// \return reference to the root node.
    Node& root(void) const { return *root_; }

    /// Check if this tree contains the scene id.
    /// \param[in] id The scene searched for.
    /// \return true if the root_ is associated with the scene id.
    bool contains_scene(int16_t id) const {
        std::lock_guard<std::mutex> lock(exec_lock_);
        return scenes_.find(id) != scenes_.cend();
    }

    Node* leaf_of_scene(int16_t id) const {
        assert(contains_scene(id));
        return scenes_.at(id);
    }

    void add_node_to_scene(int16_t id, Node* node) const {
        std::lock_guard<std::mutex> lock(exec_lock_);
        Log("SCENETREE::addNodeToScene", node->id(), "/", node->module_id(),
            " -> ", id);
        scenes_[id] = node;
    }

    void log_scenes(void) {
        Log("SCENETREE::Log", (void*)this, ": ", (void*)root_);
        if (root_)
            Log("SCENETREE::Log", (void*)this, ": ", root().id(), "/",
                root().module_id(), " (", (void*)root_, ")");
        for (auto const& scene : scenes_) {
            Log("SCENETREE::Log", scene.first, ":", scene.second->id());
        }
    }

    void execute(Node::ModuleExecutor executor, Timestamp timestamp) {
        std::lock_guard<std::mutex> lock(exec_lock_);
        root_->execute(executor, timestamp);
    }

    bool active(void) const { return active_; }
    void activate(void) { active_ = true; }
    void deactivate(void) { active_ = false; }
};

/// Administration of multiple SceneTree's.
class SceneTrees {
private:
    using Nodes = SharedResource<Node>;

    Nodes scene_nodes_;
    std::vector<SceneTree*> scene_trees_;

public:
    static_assert(noexcept(Node()), "node");
    static_assert(noexcept(std::vector<SceneTree*>()), "st");
    SceneTrees(void) noexcept(noexcept(Node()) and
                              noexcept(std::vector<SceneTree*>())) {}
    ~SceneTrees(void) {
        for (auto& tree : scene_trees_) {
            delete tree;
        }
    }

    /// Are there active nodes/scene trees?
    /// \return empty_
    bool empty(void) const { return scene_trees_.size() == 0; }

    /// \note scene_id must be unique, it is not checked here but
    /// things will break badly if it's not. Also, it is not checked
    /// here if the module for module_id does exist.
    int16_t scene_start(int16_t scene_id, int16_t module_id);

    /// Add an existing module to the end of an existing scene.
    ///  \todo This might fail if executed in quick succession of the
    /// corresponding call to scene_start, due to the root node
    /// being allocated sequentially in this thread but actually
    /// persisted in the parrallel (execution) thread. The workaround
    /// on the calling side is to introduce a short delay after scene_start().
    /// This is currently expected to be done outside of the api.
    /// \param[in] scene_id Id of an existing scene.
    /// \param[in] module_id Id of an existing module.
    /// \return
    /// - #TV_OK if good.
    /// - #TV_NODE_ALLOCATION_FAILED an error occured in SharedResource.
    /// - #TV_INVALID_ID one of both id's is invalid.
    int16_t add_to_scene(int16_t scene_id, int16_t module_id);

    void exec_all(Node::ModuleExecutor executor, Timestamp timestamp);
    void exec_scene(int16_t scene_id);

private:
    int16_t _next_node_id(void) const {
        static int16_t node_id{std::numeric_limits<int8_t>::max() + 1};
        return node_id++;
    }
};
}

#endif
