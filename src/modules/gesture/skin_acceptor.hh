/// \file skin_acceptor.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of \c Acceptors for use with FindObject.
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

#pragma once

#include "find_object.hh"
#include "hand.hh"

struct SkinAcceptor {
    virtual ~SkinAcceptor(void) = default;
    uint8_t const* image{nullptr};
    uint16_t framewidth{0};

    bool operator()(FindObject<Hand>::Thing const& thing, Hand& hand);
    virtual bool operator()(FindObject<Hand>::Thing const& thing) = 0;
};

/// Explicit skin region detection, see "A Survey on Pixel-Based Skin Color
/// Detection Techniques"
struct ExplicitSkinRegionAcceptor : public SkinAcceptor {
    bool operator()(FindObject<Hand>::Thing const& thing) override;
};
