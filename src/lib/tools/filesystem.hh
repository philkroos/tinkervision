/// \file filesystem.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Contains functions to interact with the filesystem.
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

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <thread>

/// Split the extension from a filename.
/// \param[in] filename name of a file with or without directory part.
/// \param[inout] extension The extension, if any, else empty string.
/// \return the basename of the file.
std::string strip_extension(std::string const& filename,
                            std::string& extension);

/// Return the filename without extension.
/// \param[in] filename name of a file with or without directory part.
/// \return the basename of the file.
std::string strip_extension(std::string const& filename);

/// Return the extension part of a  filename
/// \param[in] filename name of a file with or without directory part
/// \return the extension of the file. Empty string if none.
std::string extension(std::string const& filename);

/// Check if a file exists in the filesystem.  This function uses stat.
/// \param[in] fullname Full filename, including path and extension.
/// \return true if fullname is a regular file in a readable directory.
bool is_file(std::string const& fullname);

/// Check if a character device exists in the filesystem.  This function uses
/// stat. Camera devices are usually character (special) devices.
/// \param[in] fullname Full filename, including path and extension.
/// \return true if fullname is a regular file in a readable directory.
bool is_cdevice(std::string const& fullname);

/// Check if a directory exists in the filesystem.  This function uses the stat
/// function.
/// \param[in] fullname Full pathname.
/// \return true if fullname is a directory in a readable path.
bool is_directory(std::string const& fullname);

/// Retrieve a list of (all) entries in a given directory, including
/// subdirectories.
/// A filter may be defined which must return true for all files to be included
/// in the list.
/// \param[in] directory Full path.
/// \param[inout] contents The found entries in path.
/// \param[in] filter An optional filter which will be called for every entries
/// basename found in path (without path), its extension and the additional
/// information if it names a regular file.  If it shall be included in
/// contents, filter must return true. Pass \c nullptr if every entry shall be
/// included.
void list_directory_content(std::string const& directory,
                            std::vector<std::string>& contents,
                            std::function<bool(std::string const& filename,
                                               std::string const& extension,
                                               bool is_regular_file)> filter);

#endif
