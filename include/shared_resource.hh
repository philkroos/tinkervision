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

#include <mutex>
#include <unordered_map>

#include "tinkervision_defines.h"

namespace tfv {

/** Resource manager providing some (limited) thread-safety.
 */
template <typename Resource>
class SharedResource {
public:
    ~SharedResource(void) {
        for (auto const& resource : allocated_) {
            if (resource.second) delete resource.second;
        }
        for (auto const& resource : garbage_) {
            if (resource.second) delete resource.second;
        }
        for (auto const& resource : managed_) {
            if (resource.second) delete resource.second;
        }
    }

    template <typename T = Resource, typename... Args>
    void allocate(TFV_Id id, Args... args) {
        std::lock_guard<std::mutex> lock(allocation_mutex_);
        allocated_[id] = new T(args...);
    }

    // Unchecked access to managed_[id], use with care!
    void remove(TFV_Id id) {
        Resource* resource = nullptr;
        {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            resource = managed_[id];
            // managed_.erase(id);
        }
        {
            std::lock_guard<std::mutex> lock(garbage_mutex_);
            garbage_[id] = resource;
        }
    }

    void persist(void) {
        std::lock_guard<std::mutex> lock(allocation_mutex_);
        std::lock_guard<std::mutex> lock(managed_mutex_);
        if (not allocated_.empty()) {
            managed_.insert(allocated_.begin(), allocated_.end());
            allocated_.clear();
        }
    }

    void cleanup(void) {
        std::lock_guard<std::mutex> lock(garbage_mutex_);
        std::lock_guard<std::mutex> lock(managed_mutex_);
        for (auto& resource : garbage_) {
            if (resource.second) {
                managed_.erase(resource.first);
                delete resource.second;
            }
        }
        garbage_.clear();
    }

    bool managed(TFV_Id id) const {
        std::lock_guard<std::mutex> lock(managed_mutex_);
        return managed_.find(id) != managed_.end();
    }

    Resource const& operator[](TFV_Id id) const {
        std::lock_guard<std::mutex> lock(managed_mutex_);
        return *(managed_.find(id)->second);
    }

    Resource* operator[](TFV_Id id) {
        std::lock_guard<std::mutex> lock(managed_mutex_);
        return managed_[id];
    }

    std::vector<TFV_Id> managed_ids(void) const {
        std::vector<TFV_Id> ids;
        {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            for (auto const& resource : managed_) {
                ids.push_back(resource.first);
            }
        }
        return ids;
    }

private:
    using ResourceMap = std::unordered_map<TFV_Id, Resource*>;
    ResourceMap allocated_;
    ResourceMap managed_;
    ResourceMap garbage_;

    std::mutex mutable allocation_mutex_;
    std::mutex mutable garbage_mutex_;
    std::mutex mutable managed_mutex_;
};
};
