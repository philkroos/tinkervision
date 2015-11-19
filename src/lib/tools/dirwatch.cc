/// \file dirwatch.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Definition of class Dirwatch.
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

#include "dirwatch.hh"

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

#include <cassert>
#include <string>
#include <algorithm>
#include <chrono>

#include "filesystem.hh"
#include "logger.hh"

Dirwatch::Dirwatch(Callback on_change) : on_change_(on_change) {}

Dirwatch::~Dirwatch(void) { stop(); }

bool Dirwatch::watch(std::string const& directory) {
    if (not is_directory(directory)) {
        return false;
    }

    // first watch?
    if (inotify_ <= 0 and not start()) {
        return false;
    }
    assert(inotify_ > 0);

    if (watches_.find(directory) == watches_.end()) {

        auto watch = inotify_add_watch(inotify_, directory.c_str(), flags_);

        if (watch < 0) {
            tv::LogError("DIRWATCH", "Could not add watch for ", directory);
            return false;
        }

        watches_[directory] = watch;
    }

    tv::Log("DIRWATCH", "Watching ", directory);

    return true;
}

void Dirwatch::unwatch(std::string const& directory) {
    if (inotify_ <= 0) {  // must be running
        tv::LogError("DIRWATCH", "Unwatch: inotify_ invalid: ", inotify_);
        return;
    }

    auto it = watches_.find(directory);

    if (it != std::end(watches_)) {

        if (inotify_rm_watch(inotify_, watches_[directory]) < 0) {
            tv::LogError("DIRWATCH", "Could not remove watch for ", directory);
            return;
        }

        watches_.erase(it);
        tv::Log("DIRWATCH", "Unwatching ", directory);

        if (watches_.empty()) {  // no watches - don't waste runtime
            stop();
        }
    }
}

void Dirwatch::stop(void) {
    /// Stop and wait for the thread, then close inotify.
    stopped_ = true;
    inotify_thread_.join();
    close(inotify_);
    inotify_ = 0;
    tv::Log("DIRWATCH", "Stopped");
}

bool Dirwatch::start(void) {
    if (inotify_ != 0) {
        return false;
    }

    /// An inotify instance is created in non-blocking mode and polled in
    /// monitor().
    inotify_ = inotify_init1(IN_NONBLOCK);

    /*checking for error*/
    if (inotify_ < 0) {
        tv::LogError("DIRWATCH", "Inotify did not start");
        inotify_ = 0;
        return false;
    }

    /// If this is a restart, each directory that has been added with watch()
    /// will be added to the watched directories automatically.
    for (auto const& directory : watches_) {
        auto watch =
            inotify_add_watch(inotify_, directory.first.c_str(), flags_);

        if (watch <= 0) {
            tv::LogError("DIRWATCH", "Could not add watch for ", directory.first);
            return false;
        }

        watches_[directory.first] = watch;
    }

    /// The actual monitor() is started in a thread.
    stopped_ = false;
    inotify_thread_ = std::thread(&Dirwatch::monitor, this);
    tv::Log("DIRWATCH", "Start");

    return true;
}

void Dirwatch::add_watched_extension(std::string const& ext) {
    extensions_.push_back(ext);
}

void Dirwatch::monitor(void) const {

    auto const event_size = sizeof(struct inotify_event);
    // buffer space for 10 events, see man inotify
    auto const event_buffer_size = (10 * (event_size + NAME_MAX + 1));
    char buffer[event_buffer_size];

    // waiting & polling for next event
    while (not stopped_) {
        errno = 0;
        auto length = read(inotify_, buffer, event_buffer_size);

        // reading non-blocking, eagain is fine.
        if (length < 0 and errno == EAGAIN) {

            std::this_thread::sleep_for(
                std::chrono::milliseconds(check_intervall_));
            continue;
        }

        else if (length < 0) {  // should eval the error here.
            tv::LogError("DIRWATCH", "Reading from inotify: ", errno, " (",
                     strerror(errno), ")");
        }

        // process each change event
        for (int i = 0; i < length;) {
            auto event = reinterpret_cast<inotify_event*>(&buffer[i]);
            i += event_size + event->len;

            // the directory-name/watch-descriptor pair
            auto it =
                std::find_if(watches_.cbegin(), watches_.cend(),
                             [&](std::pair<std::string, int> const& entry) {
                    return entry.second == event->wd;
                });

            assert(it != watches_.cend());

            if (event->mask & IN_DELETE_SELF) {  // a watched directory removed
                on_change_(Event::DIR_DELETED, it->first, "");
            }

            else if (not event->len) {  // delete_self has no length
                continue;
            }

            // a file created/deleted/moved
            else if (event->mask & (IN_CREATE | IN_DELETE | IN_MOVE)) {

                // with extension that is being watched
                auto const ext = extension(event->name);
                if (extensions_.empty() or
                    std::find(extensions_.cbegin(), extensions_.cend(), ext) !=
                        extensions_.cend()) {

                    auto change = (event->mask & (IN_CREATE | IN_MOVED_TO)
                                       ? Event::FILE_CREATED
                                       : Event::FILE_DELETED);

                    on_change_(change, it->first, event->name);
                }
            }
        }
    }
}
