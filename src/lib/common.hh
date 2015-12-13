/// \file common.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Common header file for the library.
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

#ifndef USR_PREFIX
#error USR_PREFIX not defined
#endif

namespace tv {}

#include "tinkervision_defines.h"
#include "image.hh"
#include "filesystem.hh"
#include "bitflag.hh"
#include "exceptions.hh"
#include "strings.hh"

#ifndef WITH_LOGGER
namespace tv {
template <typename... Args>
inline void Log(char const* prefix, Args const&... args) {}
template <typename... Args>
inline void LogDebug(char const* prefix, Args const&... args) {}
template <typename... Args>
inline void LogError(char const* prefix, Args const&... args) {}
template <typename... Args>
inline void LogWarning(char const* prefix, Args const&... args) {}
}
#else
#include "logger.hh"
#endif
