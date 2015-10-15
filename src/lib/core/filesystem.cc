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

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/inotify.h>

#include <cassert>
#include <string>
#include <algorithm>
#include <chrono>

#include "logger.hh"

/*
static std::string basename(std::string const& fullname) {
    return std::string(
        std::find(fullname.rbegin(), fullname.rend(), '/').base(),
        fullname.end());
}
*/
static std::string strip_extension(std::string const& filename,
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

std::string tv::extension(std::string const& filename) {
    return std::string(
        std::find(filename.rbegin(), filename.rend(), '.').base(),
        filename.end());
}

bool tv::is_file(std::string const& fullname) {
    struct stat buffer;
    return (stat(fullname.c_str(), &buffer) == 0) and S_ISREG(buffer.st_mode);
}

bool tv::is_directory(std::string const& fullname) {
    struct stat buffer;
    return (stat(fullname.c_str(), &buffer) == 0) and S_ISDIR(buffer.st_mode);
}

void tv::list_directory_content(
    std::string const& directory, std::vector<std::string>& contents,
    std::function<bool(std::string const& filename,
                       std::string const& extension, bool is_regular_file)>
        filter) {

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

tv::Dirwatch::Dirwatch(Callback on_change) : on_change_(on_change) {}

tv::Dirwatch::~Dirwatch(void) { stop(); }

bool tv::Dirwatch::watch(std::string const& directory) {
    if (not is_directory(directory)) {
        return false;
    }

    // first watch?
    if (inotify_ <= 0 and not start()) {
        return false;
    }
    assert(inotify_ > 0);

    if (watches_.find(directory) == watches_.end()) {

        auto watch = inotify_add_watch(inotify_, directory.c_str(),
                                       IN_CREATE | IN_DELETE | IN_DELETE_SELF);

        if (watch < 0) {
            LogError("DIRWATCH", "Could not add watch for ", directory);
            return false;
        }

        watches_[directory] = watch;
    }

    Log("DIRWATCH", "Watching ", directory);

    return true;
}

void tv::Dirwatch::unwatch(std::string const& directory) {
    if (inotify_ <= 0) {  // must be running
        LogError("DIRWATCH", "Unwatch: inotify_ invalid: ", inotify_);
        return;
    }

    auto it = watches_.find(directory);

    if (it != std::end(watches_)) {

        if (inotify_rm_watch(inotify_, watches_[directory]) < 0) {
            LogError("DIRWATCH", "Could not remove watch for ", directory);
            return;
        }

        watches_.erase(it);
        Log("DIRWATCH", "Unwatching ", directory);

        if (watches_.empty()) {  // no watches - don't waste runtime
            stop();
        }
    }
}

void tv::Dirwatch::stop(void) {
    stopped_ = true;
    inotify_thread_.join();
    close(inotify_);
    inotify_ = 0;
    Log("DIRWATCH", "Stopped");
}

bool tv::Dirwatch::start(void) {
    if (inotify_ != 0) {
        return false;
    }

    inotify_ = inotify_init1(IN_NONBLOCK);

    /*checking for error*/
    if (inotify_ < 0) {
        LogError("DIRWATCH", "Inotify did not start");
        inotify_ = 0;
        return false;
    }

    for (auto const& directory : watches_) {
        auto watch = inotify_add_watch(inotify_, directory.first.c_str(),
                                       IN_CREATE | IN_DELETE | IN_DELETE_SELF);
        if (watch <= 0) {
            LogError("DIRWATCH", "Could not add watch for ", directory.first);
        }

        watches_[directory.first] = watch;
    }

    stopped_ = false;
    inotify_thread_ = std::thread(&Dirwatch::monitor, this);
    Log("DIRWATCH", "Start");

    return true;
}

void tv::Dirwatch::add_watched_extension(std::string const& ext) {
    extensions_.push_back(ext);
}

void tv::Dirwatch::monitor(void) const {

    auto const event_size = sizeof(struct inotify_event);
    auto const event_buffer_size = (1024 * (event_size + 16));
    char buffer[event_buffer_size];

    // waiting & polling for next event
    while (not stopped_) {
        auto length = read(inotify_, buffer, event_buffer_size);

        if (EAGAIN == length) {  // no data
            std::this_thread::sleep_for(
                std::chrono::milliseconds(check_intervall_));
            continue;
        }

        if (length < 0) {
            LogError("DIRWATCH", "Reading from inotify: ", length);
        }

        // process each change event
        for (auto i = 0; i < length;) {
            auto event = reinterpret_cast<inotify_event*>(&buffer[i]);
            i += event_size + event->len;

            if (not event->len) {
                continue;
            }

            // the directory-name/watch-descriptor pair
            auto it =
                std::find_if(watches_.cbegin(), watches_.cend(),
                             [&](std::pair<std::string, int> const& entry) {
                    return entry.second == event->wd;
                });

            assert(it != watches_.cend());

            if (event->mask & IN_IGNORED) {
                if (event->name == it->first) {  // a directory removed

                    on_change_(Event::DIR_DELETED, it->first, "");
                }

                // a file created or deleted
            } else if (event->mask & IN_CREATE or event->mask & IN_DELETE) {

                // with extension that is being watched
                auto const ext = extension(event->name);
                if (extensions_.empty() or
                    std::find(extensions_.cbegin(), extensions_.cend(), ext) !=
                        extensions_.cend()) {

                    auto change =
                        (event->mask & IN_CREATE ? Event::FILE_CREATED
                                                 : Event::FILE_DELETED);

                    on_change_(change, it->first, event->name);
                }
            }
        }
    }
}
