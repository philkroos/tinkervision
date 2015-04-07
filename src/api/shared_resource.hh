/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014-2015 philipp.kroos@fh-bielefeld.de

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
#include <forward_list>
#include <functional>

#include "tinkervision_defines.h"
#include "exceptions.hh"

namespace tfv {

template <typename Resource>
class SharedResource {
    using ResourceMap = std::unordered_map<TFV_Int, Resource*>;
    using ResourceList = std::forward_list<TFV_Int>;
    using Iterator = typename ResourceMap::iterator;
    using ConstIterator = typename ResourceMap::const_iterator;

public:
    using value_type = Resource;

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
     * Execute a function on all active resources in turn.
     * \parm[in] executor The function to be executed on each resource.
     */
    void exec_all(std::function<void(TFV_Int, Resource&)> executor) {
        if (not managed_.size()) {
            return;

        } else {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            for (auto& resource : managed_) {
                executor(resource.first, *resource.second);
            }
        }
    }

    /**
     * Execute a function on a single resource, given that it is active.
     *
     * \parm[in] id The id of the resource on which executor shall be executed.
     * \parm[in] executor The function to be executed on the resource identified
     * by id.
     */
    void exec_one(TFV_Int id, std::function<void(Resource&)> executor) {
        if (not managed_.size()) {
            return;
        }
        std::lock_guard<std::mutex> lock(managed_mutex_);
        auto it = managed_.find(id);
        if (it != managed_.end()) {
            executor(resource(it));
        }
    }

    /**
     * Evaluate a predicate for each active resource and counts the number of
     * true results.
     * \parm[in] predicate A predicate.
     * \return The number of true results over each active module.
     */
    size_t count(std::function<bool(Resource const&)> predicate) {
        std::lock_guard<std::mutex> lock(managed_mutex_);
        size_t count = 0;
        for (auto const& resource : managed_) {
            if (predicate(*resource.second)) {
                count++;
            }
        }
        return count;
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
    template <typename T, typename... Args>
    bool allocate(TFV_Int id, Args... args) {
        static_assert(std::is_convertible<T*, Resource*>::value,
                      "Wrong type passed to allocate");

        std::lock_guard<std::mutex> lock(allocation_mutex_);
        if (exists(allocated_, id)) {
            return false;
        }

        try {
            allocated_[id] = new T(id, args...);

        } catch (tfv::ConstructionException const& ce) {
            std::cout << ce.what() << std::endl;

            // allocated_[id] does not exist
            return false;
        }
        return true;
    }

    /**
     * Marks the resource associated with id as removable.
     * The resource is still active, i.e. managed() would return t.
     */
    void free(TFV_Int id) {
        Resource* resource = nullptr;
        {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            if (exists(managed_, id)) {
                resource = managed_[id];
                managed_.erase(id);
                ids_managed_.remove(id);
            }
        }
        if (resource) {
            std::lock_guard<std::mutex> lock(garbage_mutex_);
            garbage_[id] = resource;
        }
    }

    /**
     * Removes each (active) resource for which a given predicate holds.
     *
     * \parm[in] predicate A predicate accepting a single resource cref.
     * \return The number of resources removed.
     */
    size_t free_if(std::function<bool(Resource const& resource)> predicate) {
        std::lock_guard<std::mutex> lock(managed_mutex_);

        auto count = static_cast<size_t>(0);
        for (auto it = managed_.cbegin(); it != managed_.cend();) {

            if (predicate(resource(it))) {
                auto const resource_id = id(it);
                garbage_[resource_id] = &resource(it);
                ids_managed_.remove(resource_id);
                managed_.erase(it++);
                count++;
            } else {
                ++it;
            }
        }

        return count;
    }

    /**
     * Check whether a resource is active.
     * \return True If the resource id is active.
     */
    bool managed(TFV_Int id) const {
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
    Resource const& operator[](TFV_Int id) const {
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
    Resource* operator[](TFV_Int id) {
        Resource* resource = nullptr;
        {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            auto it = managed_.find(id);
            resource = (it == managed_.end()) ? nullptr : it->second;
        }
        return resource;
    }

    // attention: not locked.
    bool size(void) const { return managed_.size(); }

private:
    /**
     * Verbose access to the id of a resource-map.
     */
    TFV_Int id(ConstIterator it) const { return it->first; }

    /**
     * Verbose access to the resource of a resource-map.
     */
    Resource& resource(ConstIterator it) const { return *it->second; }

    /**
     * Verbose check if a  resource exists in a map.
     */
    bool exists(ResourceMap const& map, TFV_Int id) const {
        return map.find(id) != map.end();
    }

    /**
     * Activates all allocated resources.
     */
    void persist(void) {
        std::lock_guard<std::mutex> a_lock(allocation_mutex_);
        if (not allocated_.empty()) {
            std::lock_guard<std::mutex> m_lock(managed_mutex_);
            for (auto& resource : allocated_) {
                ids_managed_.push_front(resource.first);
            }
            managed_.insert(allocated_.begin(), allocated_.end());
            allocated_.clear();
        }
    }

    /**
     * Deactivates all resources marked as removable and deletes them.
     */
    void cleanup(void) {
        if (not garbage_.empty()) {
            std::lock_guard<std::mutex> g_lock(garbage_mutex_);

            for (auto& resource : garbage_) {
                if (resource.second) {
                    delete resource.second;
                }
            }
            garbage_.clear();
        }
    }

private:
    ResourceMap allocated_;     ///< After allocate(), before persist()
    ResourceMap managed_;       ///< After persist(), available for exec*
    ResourceMap garbage_;       ///< After free(), before cleanup()
    ResourceList ids_managed_;  ///< Sorted access to the active resources

    // Needing to lock these sometimes in methods that do not change internal
    // state, so declaring these mutable.
    std::mutex mutable allocation_mutex_;
    std::mutex mutable garbage_mutex_;
    std::mutex mutable managed_mutex_;
};
}
