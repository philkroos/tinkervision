/// \file skin_accoptor.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Implementation of \c Acceptors for use with FindObject.
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

#include "skin_acceptor.hh"
#include <iostream>

bool SkinAcceptor::operator()(FindObject<Hand>::Thing const& thing,
                              Hand& hand) {
    if (not(*this)(thing)) {
        return false;
    }
    return make_hand(image, framewidth, thing, hand);
}

bool ExplicitSkinRegionAcceptor::operator()(
    FindObject<Hand>::Thing const& thing) {
    if (not image) {
        return false;
    }

    uint8_t b, g, r;
    bgr_average(thing, image, b, g, r);

    if (not(r > 95 and g > 40 and b > 20 and r > (g + 15) and r > (b + 15))) {

        std::cout << "Rejecting " << (int)b << "," << (int)g << "," << (int)r
                  << std::endl;
        return false;
    }

    return true;
}
