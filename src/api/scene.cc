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

/** \file scene.cc
*/

#include <algorithm>

#include "scene.hh"

void tfv::Scene::execute(tfv::Image const& image) {
    root_.execute(image);

    for (auto const& scene : subscenes_) {
        scene->execute(image);
    }
}

/*
tfv::Scene& tfv::Scene::subscene_from_module(tfv::Module& module) {
    subscenes_.emplace_back(module);
    return subscenes_.back();
}
*/


void tfv::Scene::subscene_from_scene(tfv::Scene& scene) {

    // prevent circular graphs
    if (not scene.has_subscene(*this)) {
        subscenes_.push_back(&scene);
    }
}
