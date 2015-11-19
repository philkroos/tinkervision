/// \file scene.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Declares Scene.
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

#include <list>
#include <cassert>

#include "tinkervision_defines.h"
#include "shared_resource.hh"
#include "module_wrapper.hh"
#include "node.hh"

namespace tv {

/// A Scene is a tree of modules, held in instances of Node.
class Scene {

public:
    Scene(int16_t id, Node& root_node) : id_(id) {
        nodes_.push_back(&root_node);
    }

    ~Scene(void) = default;

    /// Depth-first execution of this scene.
    void execute(std::function<void(int16_t module_id)> executor,
                 tv::Timestamp timestamp) {
        tree().execute_for_scene(executor, timestamp, id_);
    }

    int16_t id(void) const { return id_; }

    Node& leaf(void) const {
        assert(not nodes_.empty());

        return *nodes_.back();
    }

    Node& tree(void) const {
        assert(not nodes_.empty());

        return **nodes_.begin();
    }

    void attach(Node* node) {
        auto last = nodes_.back();

        if (node != nullptr) {
            nodes_.push_back(node);
        }
        last->add_child(nodes_.back());
    }

    void disable(void) { disabled_ = true; }

    void enable(void) { disabled_ = false; }

    bool enabled(void) { return not disabled_; }

private:
    int16_t id_;
    std::list<Node*> nodes_;

    bool disabled_ = false;
};

bool operator==(Scene const& lhs, Scene const& rhs);
}
