/// \file filesystem.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Contains functions to interact with the filesystem.
///
/// This file is part of Tinkervision - Vision Library for Tinkerforge Redbrick
/// \sa https://github.com/Tinkerforge/red-brick
/// \sa filesystem.hh
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

#include "filesystem.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>

#include "logger.hh"

/*
static std::string basename(std::string const& fullname) {
    return std::string(
        std::find(fullname.rbegin(), fullname.rend(), '/').base(),
        fullname.end());
}
*/
std::string strip_extension(std::string const& filename,
                            std::string& extension) {
    auto rev_it = std::find(filename.rbegin(), filename.rend(), '.');

    if (rev_it == filename.rend() or
        rev_it == filename.rend() - 1) {  // no extension or dotfile
        extension = "";
        return filename;
    }

    extension = std::string(rev_it.base(), filename.cend());
    return std::string(filename.cbegin(), rev_it.base() - 1);
}

std::string strip_extension(std::string const& filename) {
    auto rev_it = std::find(filename.rbegin(), filename.rend(), '.');

    if (rev_it == filename.rend() or
        rev_it == filename.rend() - 1) {  // no extension or dotfile
        return filename;
    }

    return std::string(filename.cbegin(), rev_it.base() - 1);
}

std::string extension(std::string const& filename) {
    return std::string(
        std::find(filename.rbegin(), filename.rend(), '.').base(),
        filename.end());
}

bool is_file(std::string const& fullname) {
    struct stat buffer;
    return (stat(fullname.c_str(), &buffer) == 0) and S_ISREG(buffer.st_mode);
}

bool is_cdevice(std::string const& fullname) {
    struct stat buffer;
    return (stat(fullname.c_str(), &buffer) == 0) and S_ISCHR(buffer.st_mode);
}

bool is_directory(std::string const& fullname) {
    struct stat buffer;
    return (stat(fullname.c_str(), &buffer) == 0) and S_ISDIR(buffer.st_mode);
}

void list_directory_content(std::string const& directory,
                            std::vector<std::string>& contents,
                            std::function<bool(std::string const& filename,
                                               std::string const& extension,
                                               bool is_regular_file)> filter) {

    if (not is_directory(directory)) {
        return;
    }

    auto dir = opendir(directory.c_str());

    for (auto entry = readdir(dir); entry != NULL; entry = readdir(dir)) {
        std::string extension;

        if (not filter or
            filter(strip_extension(entry->d_name, extension), extension,
                   is_file(directory + "/" + entry->d_name))) {

            contents.push_back(entry->d_name);
        }
    }
}
