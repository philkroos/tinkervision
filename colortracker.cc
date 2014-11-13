/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014 philipp.kroos@fh-bielefeld.de

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

#include "colortracker.hh"

#ifdef DEV
#include <thread>
#include <chrono>
#endif

void tfv::Colortracker::track(tfv::Frame const& frame, TFV_Byte min_hue,
                              TFV_Byte max_hue) const {
#ifdef DEV
    std::cout << "Tracking with values " << std::to_string(min_hue) << ","
              << std::to_string(max_hue) << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif  // DEV
}

void tfv::Colortracker::on_execute(tfv::Frame const& frame,
                                   Feature const& configuration) {

    track(frame, configuration.min_hue, configuration.max_hue);
    // configuration->callback
}

void tfv::Colortracker::after_configuration_added(
    Feature const& configuration) {
    std::cout << "Added tracker for cam " << configuration.camera_id
              << std::endl
              << " ---> min/max-hue: " << std::to_string(configuration.min_hue)
              << "/" << std::to_string(configuration.max_hue) << std::endl;
}

void tfv::Colortracker::after_configuration_removed(
    Feature const& configuration) {
    std::cout << "Removed tracker for cam " << configuration.camera_id
              << std::endl
              << " ---> min/max-hue: " << std::to_string(configuration.min_hue)
              << "/" << std::to_string(configuration.max_hue) << std::endl;
}
