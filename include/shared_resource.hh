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
#include <functional>

#include "tinkervision_defines.h"

namespace tfv {

/**
 * Map-based RAII-style resource manager providing (limited) thread-safety.
 *
 * Provides a manager for a pool of resources which are to be used in
 * multithreaded code.  Held resources are in one of the three states
 * allocated (after construction by a call to allocate()), active
 * (from allocated during an exec_all() or exec_one()) and removed
 * (from managed during an exec_all() or exec_one() if free() was
 * called for the resource).  allocate(), free(), with exec_all() and
 * exec() are thread-safe.  The latter are expected to be used to
 * execute code on the managed resources.  Resources are stored in a
 * map under an id and are retrievable by this id via the not
 * thread-safe operator[]-overloads.
 *
 * \see operator[]() for limitations of the thread-safety.
 */
template <typename Resource>
class SharedResource {
    using ResourceMap = std::unordered_map<TFV_Id, Resource*>;
    using Iterator = typename ResourceMap::iterator;
    using ConstIterator = typename ResourceMap::const_iterator;

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

    /**
     * Executes a function on all active resources in turn.
     */
    void exec_all(std::function<void(TFV_Id, Resource&)> executor) {
        if (not managed_.size()) {
            return;
        }
        std::lock_guard<std::mutex> lock(managed_mutex_);
        for (auto& resource : managed_) {
            executor(resource.first, *resource.second);
        }
    }

    /**
     * Executes a function on a single resource, given that it is active.
     *
     * \return t if the resource was active.
     */
    void exec_one(TFV_Id id, std::function<void(Resource&)> executor) {
        if (not managed_.size()) {
            return;
        }
        std::lock_guard<std::mutex> lock(managed_mutex_);
        auto it = managed_.find(id);
        if (it != managed_.end()) {
            executor(resource(it));
        }
        return it != managed_.end();
    }

    void update(void) {
        persist();
        cleanup();
    }

    /**
     * Constructs a Resource with the arguments ...args.  Prior to
     * construction, it is checked if another resource is allocated
     * under the same id.  Only the inactive resources are considered
     * here: If a resource with the given id is already activated, the
     * new resource will be constructed but never be activated.
     * Therefore, it is mandatory to always provide a fresh id.
     *
     * \return false if the id was already allocated.
     */
    template <typename T = Resource, typename... Args>
    bool allocate(TFV_Id id, Args... args) {
        std::lock_guard<std::mutex> lock(allocation_mutex_);
        if (exists(allocated_, id)) {
            return false;
        }
        allocated_[id] = new T(args...);
        return true;
    }

    /**
     * Marks the resource associated with id as removable.
     * The resource is still active, i.e. managed() would return t.
     */
    void free(TFV_Id id) {
        Resource* resource = nullptr;
        {
            std::cout << "Waiting for managed_mutex_..." << std::endl;
            std::lock_guard<std::mutex> lock(managed_mutex_);
            std::cout << "Got managed_mutex_." << std::endl;
            if (exists(managed_, id)) {
                resource = managed_[id];
            }
        }
        if (resource) {
            std::lock_guard<std::mutex> lock(garbage_mutex_);
            garbage_[id] = resource;
        }
    }

    /**
     * Check whether a resource is active.
     * \return True If the resource id is active.
     */
    bool managed(TFV_Id id) const {
        std::lock_guard<std::mutex> lock(managed_mutex_);
        return exists(managed_, id);
    }

    /**
     * Unchecked access to the active resources.  The
     * operator[]-method provides access to a managed resource.  Usage
     * of the retrieved resource reference is not thread-safe.  I.e.,
     * during usage of the resource it must not be free()'d and there
     * must not be a concurrent thread calling exec_all().
     *
     * \todo Mutex acquisition is not fair and so this method takes
     * too long sometimes.
     */
    Resource const& operator[](TFV_Id id) const {
        ConstIterator it;
        {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            it = managed_.find(id);
        }
        return *(it->second);
    }

    /**
     * Access to the active resources.  Returns nullptr if no resource
     * is found under the give id.  The operator[]-method provides
     * access to a managed resource.  Usage of the retrieved resource
     * pointer is not thread-safe.  I.e., during usage of the resource
     * it must not be free()'d and there must not be a concurrent
     * thread calling exec_all().
     *
     * \todo Mutex acquisition is not fair and so this method takes
     * too long sometimes.
     */
    Resource* operator[](TFV_Id id) {
        Resource* resource = nullptr;
        {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            auto it = managed_.find(id);
            resource = (it == managed_.end()) ? nullptr : it->second;
        }
        return resource;
    }

private:
    /**
     * Verbose access to the id of a resource-map.
     */
    TFV_Id id(ConstIterator it) const { return it->first; }

    /**
     * Verbose access to the resource of a resource-map.
     */
    TFV_Id resource(ConstIterator it) const { return it->second; }

    /**
     * Verbose check if a  resource exists in a map.
     */
    bool exists(ResourceMap const& map, TFV_Id id) const {
        return map.find(id) != map.end();
    }

    /**
     * Activates all allocated resources.
     */
    void persist(void) {
        std::lock_guard<std::mutex> a_lock(allocation_mutex_);
        if (not allocated_.empty()) {
            std::lock_guard<std::mutex> m_lock(managed_mutex_);
            managed_.insert(allocated_.begin(), allocated_.end());
            allocated_.clear();
        }
    }

    /**
     * Deactivates all resources marked as removable and deletes them.
     */
    void cleanup(void) {
        std::lock_guard<std::mutex> g_lock(garbage_mutex_);
        for (auto& resource : garbage_) {
            if (resource.second) {
                std::lock_guard<std::mutex> m_lock(managed_mutex_);
                managed_.erase(resource.first);
                delete resource.second;
            }
        }
        garbage_.clear();
    }

    // /**
    //  * Returns a list of all active resources.
    //  */
    // std::vector<TFV_Id> managed_ids(void) const {
    //     std::vector<TFV_Id> ids;
    //     {
    //         std::lock_guard<std::mutex> lock(managed_mutex_);
    //         for (auto const& resource : managed_) {
    //             ids.push_back(resource.first);
    //         }
    //     }
    //     return ids;
    // }

private:
    ResourceMap allocated_;
    ResourceMap managed_;
    ResourceMap garbage_;
    bool raw_access_ = false;

    std::mutex mutable allocation_mutex_;
    std::mutex mutable garbage_mutex_;
    std::mutex mutable managed_mutex_;
};
};
