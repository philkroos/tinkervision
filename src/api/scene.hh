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

/** \file scene.hh

    A Scene is a tree of modules, held in instances of Node.

    \todo The usual methods for a tree: depth, size, leaves, ...
*/

#include <vector>

#include "module.hh"
#include "node.hh"

namespace tfv {
    class Scene;
    using Successors = std::vector<Scene*>;

    class Scene {
    public:
        explicit Scene(Module& root_module) : root_(Node(root_module)) {}

        /**
         * Depth-first execution of this scene.
         */
        void execute(Image const& image);

        //        void subscene_from_module(Module& module);
        void subscene_from_scene(Scene& scene);

    private:
        Node root_;
        Successors subscenes_;

        bool has_subscene(Scene const& scene) const {
            return std::find_if(subscenes_.cbegin(), subscenes_.cend(),
                                [&scene] (Scene* s) {
                                    return &scene == s;
                                }
                                ) != subscenes_.cend();
        }
    };
}
