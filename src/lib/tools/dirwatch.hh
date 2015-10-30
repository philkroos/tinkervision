/// \file dirwatch.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of class Dirwatch.
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

#ifndef DIRWATCH_H
#define DIRWATCH_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <thread>

#include <sys/inotify.h>

namespace tv {

/// Monitor one or more directories for changes.
/// The list of events registered is enumerated in Event.
/// To use this class, first instantiate it with a callback which will be called
/// for each change in the watched directories. Optionally add one or more
/// extensions using only_extension(), which will reduce notifications to events
/// involving corresponding files. (If a monitored directory is being deleted, a
/// notification will be sent no matter what.)  Finally, add one or more
/// directories to watch().  This class polls the inotify interface to learn
/// about directory changes.
class Dirwatch {
public:
    /// Flags for the events handled here.
    enum class Event : uint8_t { FILE_CREATED, FILE_DELETED, DIR_DELETED };

    /// Signature for a static callback function
    using Callback = std::function<
        void(Event event, std::string const& dir, std::string const& file)>;

private:
    std::map<std::string, int>
        watches_;  ///< Watched directory, inotify watch descriptor
    std::vector<std::string> extensions_;  ///< List of respected extensions

    uint32_t flags_ = IN_CREATE | IN_MOVE | IN_DELETE | IN_DELETE_SELF;

    int mutable inotify_{0};  ///< File descriptor for inotify instance
    std::thread mutable inotify_thread_;  ///< Asynchronously polling for
    /// inotify events
    bool stopped_{true};              ///< Signal for thread to halt
    uint16_t check_intervall_{1000};  ///< Polling intervall

    Callback on_change_;  ///< Called on change.

    /// Thread polling inotify.
    void monitor(void) const;

public:
    /// Default c'tor.
    /// Expects a function which will be called for each change in the watched
    /// directories.
    explicit Dirwatch(Callback on_change);

    /// Variant c'tor.
    /// Expects a member function and a class instance.
    template <class Callable>
    Dirwatch(void (Callable::*on_change)(Event event, std::string const& dir,
                                         std::string const& file),
             Callable* object) {

        on_change_ = std::bind(on_change, object, std::placeholders::_1,
                               std::placeholders::_2, std::placeholders::_3);
    }

    /// Standard d'tor.
    ~Dirwatch(void);

    /// Add a directory to be watched.
    /// \param[in] directory full path name.
    /// \return true if the directory exists and is being watched now.
    bool watch(std::string const& directory);

    /// Unwatch a directory.
    /// \param[in] directory full path name.
    void unwatch(std::string const& directory);

    /// Halt execution.
    /// Do not modify the list of watched directories or extensions, but stop
    /// watching for changes.
    void stop(void);

    /// (Re-)Start execution.
    /// \return true if the execution has resumed successfully.
    bool start(void);

    /// Set an intervall for the polling of inotify. This intervall will only be
    /// used if a read from inotify does not return any changes. I.e., if a
    /// change has been detected, inotify will be polled again immediately after
    /// the list of changes has been handled.
    /// \param[in] milliseconds Intervall in milliseconds between polling.
    void set_polling_intervall(uint16_t milliseconds) {
        check_intervall_ = milliseconds;
    }

    /// Add a file extension to be respected during watching of changes. Once an
    /// extension has been registered, only changes on matching files will be
    /// handled (plus deletion of the watched directories themselves).  If no
    /// extension has been set, all files will be handled.
    /// \param[in] ext Extension without the dot, i.e. \c "so" for shared
    /// objects.
    void add_watched_extension(std::string const& ext);

    /// Reset the list of respected extensions. Effectively, watch changes on
    /// every file in the registered directories.
    void reset_extension_filter(void) { extensions_.clear(); }
};
}

#endif
