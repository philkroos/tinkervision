/// \file shared_resource.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Declares and defines a RAII-style container.
///
/// SharedResource was created to manage the lifecycle of loaded modules in the
/// library in a thread-safe way.  A lot of the initial functionality has now
/// been removed, most notably the different stati of modules.  Initially,
/// modules could be in three different stages: allocated/managed/garbage.  The
/// execution of the main loop (\see Api::execute()) was fully multi-threaded,
/// in a way that modules could be loaded during one execution cycle.  To
/// prevent access to the list of active modules, they were held in an allocated
/// status until the execution cycle completed, then they were persisted.
/// Removal worked in the same way, the other way around.  It was done like that
/// to have an initial response for the user, who now has to wait until after
/// the current exec-cycle, since this class locks access to the list of modules
/// during each cycle.  The initial idea got to complicated, since a valid
/// response for a 'module-load'-call did not mean that the module is actually
/// available.  The initial purpose of this class changed around commit b8df857.
/// It still serves as a basic RAII-style container and can be improved upon if
/// necessary.
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

#ifndef SHARED_RESOURCE_H
#define SHARED_RESOURCE_H

#include <mutex>
#include <unordered_map>
#include <forward_list>
#include <algorithm>
#include <functional>

#include "tinkervision_defines.h"
#include "exceptions.hh"
#include "logger.hh"

namespace tv {

template <typename Resource>
class SharedResource {

public:
    using value_type = Resource;
    using ResourceMap = std::unordered_map<int16_t, value_type*>;
    using IdList = std::forward_list<int16_t>;

    using ExecAll = std::function<void(int16_t, Resource&)>;
    using ExecOne = std::function<int16_t(Resource&)>;

    using AfterAllocatedHook = std::function<void(Resource&)>;

    using Allocator = std::function<void(void)>;
    using Deallocator = std::function<void(Resource&)>;

private:
    struct ResourceContainer {
        Resource* resource = nullptr;
        Deallocator deallocator;

        ResourceContainer(void) noexcept = default;
        ResourceContainer(Resource* resource, Deallocator deallocator) noexcept
            : resource(resource),
              deallocator(deallocator) {}
    };

    using ResourceContainerMap = std::unordered_map<int16_t, ResourceContainer>;
    using Iterator = typename ResourceContainerMap::iterator;
    using ConstIterator = typename ResourceContainerMap::const_iterator;

public:
    ~SharedResource(void) {
        for (auto const& resource : managed_) {
            if (resource.second.resource) delete resource.second.resource;
        }
    }

    /// Execute a function on all active resources in turn.
    /// \parm[in] executor The function to be executed on each resource.
    void exec_all(ExecAll executor) {
        if (not managed_.size()) {
            return;

        } else {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            for (auto& id : ids_managed_) {
                executor(id, *(managed_[id].resource));
            }
            /*
                        for (auto& resource : managed_) {
                            executor(resource.first, *resource.second);
                        }
        */
        }
    }

    /// Execute a function on a single resource, given that it is active.
    /// \param[in] id The id of the resource on which executor shall be
    /// executed.
    /// \param[in] executor The function to be executed on the resource
    /// identified by id.
    int16_t exec_one(int16_t id, ExecOne executor) {
        if (not managed_.size()) {
            return TV_INVALID_ID;
        }

        std::lock_guard<std::mutex> lock(managed_mutex_);
        auto it = managed_.find(id);
        if (it != managed_.end()) {
            return executor(resource(it));
        } else {
            return TV_INVALID_ID;
        }
    }

    /// Evaluate a predicate for each active resource and counts the number of
    /// true results.
    /// \param[in] predicate A predicate.
    /// \return The number of true results over each active module.
    size_t count(std::function<bool(Resource const&)> predicate) {
        std::lock_guard<std::mutex> lock(managed_mutex_);
        size_t count = 0;
        for (auto const& resource : managed_) {
            if (predicate(*(resource.second.resource))) {
                count++;
            }
        }
        return count;
    }

    bool insert(int16_t id, Resource* module, Deallocator deallocator) {

        std::lock_guard<std::mutex> lock(managed_mutex_);
        if (exists(managed_, id)) {
            LogWarning("SHARED_RESOURCE", "Double allocate");

            return false;
        }

        managed_[id] = {module, deallocator};
        ids_managed_.push_front(id);
        Log("SHARED_RESOURCE", "Inserted ", module->name(), " (", id, ")");
        return true;
    }

    bool remove(int16_t id) {
        std::lock_guard<std::mutex> lock(managed_mutex_);
        if (not exists(managed_, id)) {
            LogWarning("SHARED_RESOURCE::remove", "Non existing");

            return false;
        }

        if (managed_[id].deallocator) {
            Log("SHARED_RESOURCE::remove", "Id ", id);
            managed_[id].deallocator(*managed_[id].resource);
        }

        managed_.erase(id);
        ids_managed_.remove(id);
        return true;
    }

    /// Constructs a Resource with the arguments ...args.  Prior to
    /// construction, it is checked if another resource is allocated
    /// under the same id.  Only the inactive resources are considered
    /// here: If a resource with the given id is already activated, the
    /// new resource will be constructed but never be activated.
    /// Therefore, it is mandatory to always provide a fresh id.
    /// \return false if the id was already allocated.
    template <typename T, typename... Args>
    bool allocate(int16_t id, AfterAllocatedHook callback, Args... args) {
        static_assert(std::is_convertible<T*, Resource*>::value,
                      "Wrong type passed to allocate");

        std::lock_guard<std::mutex> lock(managed_mutex_);
        if (exists(managed_, id)) {
            LogWarning("SHARED_RESOURCE::allocate", "Double allocate");

            return false;
        }

        try {
            managed_[id] = ResourceContainer();
            managed_[id].resource = new T(id, args...);
            ids_managed_.push_front(id);

        } catch (tv::ConstructionException const& ce) {
            LogError("SHARED_RESOURCE::allocate", ce.what());

            return false;
        }

        if (nullptr != callback) {
            callback(*managed_[id].resource);
        }

        return true;
    }

    /// Marks the resource associated with id as removable.
    /// The resource is still active, i.e. managed() would return t.
    void free(int16_t id) {
        // Resource* resource = nullptr;
        {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            if (exists(managed_, id)) {
                auto resource = managed_[id].resource;
                managed_.erase(id);
                ids_managed_.remove(id);
                delete resource;
            }
        }
        /*
            if (resource) {
                std::lock_guard<std::mutex> lock(garbage_mutex_);
                garbage_[id] = resource;
            }
        */
    }

    /// Removes each (active) resource for which a given predicate holds.
    /// \parm[in] predicate A predicate accepting a single resource cref.
    /// \return The number of resources removed.
    size_t free_if(std::function<bool(Resource const& resource)> predicate) {
        std::lock_guard<std::mutex> mlock(managed_mutex_);
        // std::lock_guard<std::mutex> glock(garbage_mutex_);

        auto count = static_cast<size_t>(0);
        for (auto it = managed_.cbegin(); it != managed_.cend();) {

            if (predicate(resource(it))) {
                // auto const resource_id = id(it);
                // garbage_[resource_id] = &resource(it);
                ids_managed_.remove(id(it));
                managed_.erase(it++);
                count++;
            } else {
                ++it;
            }
        }

        return count;
    }

    void free_all(void) {
        std::lock_guard<std::mutex> mlock(managed_mutex_);
        // std::lock_guard<std::mutex> glock(garbage_mutex_);
        // garbage_.insert(managed_.begin(), managed_.end());
        managed_.clear();
        ids_managed_.clear();
    }

    /// Check whether a resource is active.
    /// \return True If the resource id is active.
    bool managed(int16_t id) const {
        std::lock_guard<std::mutex> lock(managed_mutex_);
        return exists(managed_, id);
    }

    /// Unchecked access to the active resources.  The
    /// operator[]-method provides access to a managed resource.  Usage
    /// of the retrieved resource reference is not thread-safe.  I.e.,
    /// during usage of the resource it must not be free()'d and there
    /// must not be a concurrent thread calling exec_all().
    /// \todo Mutex acquisition is not fair and so this method takes
    /// too long sometimes.
    Resource const& operator[](int16_t id) const {
        ConstIterator it;
        {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            it = managed_.find(id);
        }
        return *(it->second.resource);
    }

    /// Access to the active resources.  Returns nullptr if no resource
    /// is found under the give id.  The operator[]-method provides
    /// access to a managed resource.  Usage of the retrieved resource
    /// pointer is not thread-safe.  I.e., during usage of the resource
    /// it must not be free()'d and there must not be a concurrent
    /// thread calling exec_all().
    /// \todo Mutex acquisition is not fair and so this method takes
    /// too long sometimes.
    Resource* operator[](int16_t id) {
        Resource* resource = nullptr;
        {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            auto it = managed_.find(id);
            resource = (it == managed_.end()) ? nullptr : it->second.resource;
        }
        return resource;
    }

    /// Like std::find_if, but over all managed resources and returning
    /// the resource pointer directly or a nullptr.
    Resource* find_if(std::function<bool(Resource const&)> unaryp) {
        Resource* resource = nullptr;
        {
            std::lock_guard<std::mutex> lock(managed_mutex_);
            auto it = std::find_if(
                managed_.begin(), managed_.end(),
                [&unaryp](std::pair<int16_t, Resource*> const& thing) {
                    return unaryp(*thing.second.resource);
                });
            resource = (it == managed_.end()) ? nullptr : it->second.resource;
        }
        return resource;
    }

    /// Define an order between two managed ids
    /// \param[in] first The id to be executed first
    /// \param[in] second The id to be executed second
    /// \return True if both id's are managed, in which case first
    /// will be executed first afterwards.
    bool sort(int8_t first, int8_t second) {
        if (not managed(first) or not managed(second)) {
            return false;
        }

        std::lock_guard<std::mutex> lock(managed_mutex_);
        auto it_second =
            std::find(std::begin(ids_managed_), std::end(ids_managed_), second);
        auto it_first =
            std::find(std::begin(ids_managed_), std::end(ids_managed_), first);

        if (it_second != std::begin(ids_managed_)) {
            ids_managed_.erase_after(std::prev(it_second));
        } else {
            ids_managed_.erase_after(ids_managed_.before_begin());
        }

        ids_managed_.insert_after(it_first, second);
        return true;
    }

    /// Manually sort the complete list of managed id's
    /// \param[in] sorter A function sorting the list of managed ids
    void sort_manually(std::function<void(IdList& ids)> sorter) {
        std::lock_guard<std::mutex> lock(managed_mutex_);
        sorter(ids_managed_);
    }

    // attention: The following not locked.
    Resource* access_unlocked(int16_t id) {
        auto it = managed_.find(id);
        return (it == managed_.end()) ? nullptr : it->second.resource;
    }

    /// Get the number of managed objects.
    size_t size(void) const { return managed_.size(); }

    /// Get the id of a managed object.
    /// \todo Need to lock?
    /// \return Id if object is in range \c [0, size())
    int16_t managed_id(size_t object) const {
        if (object >= size()) {
            return TV_INVALID_ID;
        }
        auto it = ids_managed_.cbegin();
        for (size_t i = 0; i < object; ++i, ++it)
            ;
        return *it;
    }

private:
    /// Verbose access to the id of a resource-map.
    int16_t id(ConstIterator it) const { return it->first; }

    /// Verbose access to the resource of a resource-map.
    Resource& resource(ConstIterator it) const { return *it->second.resource; }

    /// Verbose check if a  resource exists in a map.
    bool exists(ResourceContainerMap const& map, int16_t id) const {
        return map.find(id) != map.end();
    }

private:
    ResourceContainerMap managed_;
    // Dynamic allocation because default constructor not nothrow
    IdList ids_managed_;  ///< Sorted access to the active resources

    std::mutex mutable managed_mutex_;
};
}

#endif
