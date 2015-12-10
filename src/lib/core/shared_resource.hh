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

#ifdef __STDC_NO_ATOMICS__
#error "Atomics not available"
#endif

#include <shared_mutex>
#include <atomic>
#include <unordered_map>
#include <list>
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
    using IdList = std::list<int16_t>;

    using ExecAll = std::function<void(int16_t, Resource&)>;
    using ExecOne = std::function<int16_t(Resource&)>;

    using CRefPredicate = std::function<bool(Resource const&)>;
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
    SharedResource(void)
        : SharedResource(&SharedResource::fallback_executor, this) {}

    template <class Callable>
    SharedResource(void (Callable::*default_executor)(int16_t, Resource&),
                   Callable* object) {

        executor_ = std::bind(default_executor, object, std::placeholders::_1,
                              std::placeholders::_2);
    }

    ~SharedResource(void) {
        for (auto const& resource : managed_) {
            if (resource.second.resource) delete resource.second.resource;
        }
    }

    /// Execute a function on all active resources in turn. The parameter is an
    /// optional replacement for the default executor set during construction.
    /// \param[in] executor The function to be executed on each resource.
    void exec_all(ExecAll executor) {
        if (not managed_.size()) {
            return;

        } else {
            std::shared_lock<std::shared_timed_mutex> lock(mutex_);

            auto limit = ids_managed_.size();
            auto id = ids_managed_.cbegin();
            for (size_t i = 0; i < limit; ++i) {
                /// but allow execution of an interrupt from exec_one_now()
                if (interrupt_lock_.test_and_set(std::memory_order_acquire)) {
                    while (
                        interrupt_lock_.test_and_set(std::memory_order_acquire))
                        ;
                }
                executor(*id, *(managed_[*id++].resource));

                interrupt_lock_.clear();
            }
        }
    }

    /// Call exec_all(ExecAll) with the default executor supplied during
    /// construction.
    void inline exec_all(void) { exec_all(executor_); }

    /// Execute a function on all active resources that satisfy a predicate..
    /// \param[in] executor The function to be executed on each resource.
    /// \param[in] predicate A predicate accepting a single resource cref.
    void exec_if(ExecAll executor, CRefPredicate predicate) {
        if (not managed_.size()) {
            return;

        } else {
            std::shared_lock<std::shared_timed_mutex> lock(mutex_);

            auto limit = ids_managed_.size();
            auto id = ids_managed_.cbegin();
            for (size_t i = 0; i < limit; ++i) {
                /// Allow interrupting execution of a specific resource.
                /// \see exec_one_now(), exec_one_now_restarting()
                while (
                    interrupt_lock_.test_and_set(std::memory_order_acquire)) {
                    if (not resume_on_interrupt_) {
                        return;
                    }
                }
                if (predicate(*(managed_[*id].resource))) {
                    executor(*id, *(managed_[*id].resource));
                }
                id++;

                interrupt_lock_.clear();
            }
        }
    }

    /// Execute a function on a single resource, given that it is active.
    /// \param[in] id The id of the resource on which executor shall be
    /// executed.
    /// \param[in] executor The function to be executed on the resource
    /// identified by id.
    /// \deprecated Favor exec_one_now for faster response.
    int16_t exec_one(int16_t id, ExecOne executor) {
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);

        if (not managed_.size()) {
            return TV_INVALID_ID;
        }

        auto it = managed_.find(id);
        if (it != managed_.end()) {
            return executor(resource(it));
        } else {
            return TV_INVALID_ID;
        }
    }

    /// Force execution of a specific ressource now.
    /// This will interrupt possible running calls to exec_if() or exec_all().
    /// \param[in] id The id of the resource on which executor shall be
    /// executed.
    /// \param[in] executor The function to be executed on each resource.
    int16_t exec_one_now(int16_t id, ExecOne executor) const {
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);

        if (not managed_.size()) {
            return TV_INVALID_ID;
        }

        resume_on_interrupt_ = true;
        return exec_one_now_common(id, executor);
    }

    /// Force execution of a specific ressource now, terminating the main loop.
    /// This is just like exec_one_now(), but it will signal the loop run from
    /// exec_all() to not resume execution but terminate.
    /// \param[in] id The id of the resource on which executor shall be
    /// executed.
    /// \param[in] executor The function to be executed on each resource.
    int16_t exec_one_now_restarting(int16_t id, ExecOne executor) const {
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);

        if (not managed_.size()) {
            return TV_INVALID_ID;
        }

        resume_on_interrupt_ = false;
        return exec_one_now_common(id, executor);
    }

    /// Interrupt the main execution loop (exec_all()) non resuming.
    void interrupt(void) {
        resume_on_interrupt_ = false;
        while (interrupt_lock_.test_and_set(std::memory_order_acquire))
            ;
        interrupt_lock_.clear();
    }

    /// Evaluate a predicate for each active resource and counts the number of
    /// true results.
    /// \param[in] predicate A predicate.
    /// \return The number of true results over each active module.
    size_t count(CRefPredicate predicate) {
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);

        size_t count = 0;
        for (auto const& resource : managed_) {
            if (predicate(*(resource.second.resource))) {
                count++;
            }
        }
        return count;
    }

    /// Insert a new resource.
    /// \param[in] id Identifier of the resource.
    /// \param[in] module resource
    /// \param[in] deallocator Optional function to be called immediately before
    /// this resource is removed.
    bool insert(int16_t id, Resource* module, Deallocator deallocator) {
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);

        if (exists(managed_, id)) {
            LogWarning("SHARED_RESOURCE", "Double allocate");

            return false;
        }

        managed_[id] = {module, deallocator};
        ids_managed_.push_back(id);
        Log("SHARED_RESOURCE", "Inserted ", module->name(), " (", id, ")");
        return true;
    }

    bool remove(int16_t id) {
        std::unique_lock<std::shared_timed_mutex> lock(mutex_);

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

    /// Removes each (active) resource for which a given predicate holds.
    /// \param[in] predicate A predicate accepting a single resource cref.
    /// \return The number of resources removed.
    size_t remove_if(std::function<bool(Resource const& resource)> predicate) {
        std::unique_lock<std::shared_timed_mutex> lock(mutex_);

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

    void remove_all(void) {
        std::unique_lock<std::shared_timed_mutex> lock(mutex_);

        managed_.clear();
        ids_managed_.clear();
    }

    /// Check whether a resource is active.
    /// \return True If the resource id is active.
    bool managed(int16_t id) const {
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);

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
            std::shared_lock<std::shared_timed_mutex> lock(mutex_);
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
            std::shared_lock<std::shared_timed_mutex> lock(mutex_);
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

            std::shared_lock<std::shared_timed_mutex> lock(mutex_);
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

        std::shared_lock<std::shared_timed_mutex> lock(mutex_);

        auto it_second =
            std::find(std::begin(ids_managed_), std::end(ids_managed_), second);
        auto it_first =
            std::find(std::begin(ids_managed_), std::end(ids_managed_), first);

        if (it_second != std::begin(ids_managed_)) {
            ids_managed_.erase(it_second);
        } else {
            ids_managed_.pop_front();
        }

        if (it_first != std::end(ids_managed_)) {
            ++it_first;
        }
        ids_managed_.insert(it_first, second);
        return true;
    }

    /// Manually sort the complete list of managed id's
    /// \param[in] sorter A function sorting the list of managed ids
    void sort_manually(std::function<void(IdList& ids)> sorter) {
        std::unique_lock<std::shared_timed_mutex> lock(mutex_);
        sorter(ids_managed_);
    }

    /// Get the number of managed objects.
    /// \return managed_.size()
    size_t size(void) const {
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);
        return managed_.size();
    }

    /// Get the id of a managed object.
    /// \return Id if object is in range \c [0, size())
    int16_t managed_id(size_t object) const {
        if (object >= size()) {
            return TV_INVALID_ID;
        }

        std::shared_lock<std::shared_timed_mutex> lock(mutex_);
        auto it = ids_managed_.cbegin();
        for (size_t i = 0; i < object; ++i, ++it)
            ;
        return *it;
    }

    /// Unlocked access. Must only be used from locking contexts (which is,
    /// every other accessing method).
    /// \param[in] id Resource id
    Resource* access_unlocked(int16_t id) {
        auto it = managed_.find(id);
        return (it == managed_.end()) ? nullptr : it->second.resource;
    }

    /// Constructs a Resource with the arguments ...args.  Prior to
    /// construction, it is checked if another resource is allocated
    /// under the same id.  Only the inactive resources are considered
    /// here: If a resource with the given id is already activated, the
    /// new resource will be constructed but never be activated.
    /// Therefore, it is mandatory to always provide a fresh id.
    /// \return false if the id was already allocated.
    /// \deprecated Use insert.
    template <typename T, typename... Args>
    bool allocate(int16_t id, AfterAllocatedHook callback, Args... args) {
        static_assert(std::is_convertible<T*, Resource*>::value,
                      "Wrong type passed to allocate");

        std::unique_lock<std::shared_timed_mutex> lock(mutex_);

        if (exists(managed_, id)) {
            LogWarning("SHARED_RESOURCE::allocate", "Double allocate");

            return false;
        }

        try {
            managed_[id] = ResourceContainer();
            managed_[id].resource = new T(id, args...);
            ids_managed_.push_back(id);

        } catch (tv::ConstructionException const& ce) {
            LogError("SHARED_RESOURCE::allocate", ce.what());

            return false;
        }

        if (nullptr != callback) {
            callback(*managed_[id].resource);
        }

        return true;
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

    /// Helper for the exec_one_now methods
    int16_t inline exec_one_now_common(int16_t id, ExecOne executor) const {

        auto result = TV_INVALID_ID;
        auto it = managed_.find(id);

        if (it != managed_.end()) {
            while (interrupt_lock_.test_and_set(std::memory_order_acquire))
                ;
            result = executor(resource(it));
            interrupt_lock_.clear();
        }

        return result;
    }

private:
    ResourceContainerMap managed_;  ///< Active resources
    IdList ids_managed_;            ///< Sorted access to the active resources

    std::shared_timed_mutex mutable mutex_;  ///< multiple reads, one write

    std::atomic_flag mutable interrupt_lock_{
        ATOMIC_FLAG_INIT};  ///< Signal to interrupt execution loops

    bool mutable resume_on_interrupt_{
        false};  ///< Signal to resume execution on interrupt

    ExecAll executor_;  ///< Default executor (for exec_all())

    /// Execute if no executor is available during exec_all().
    /// An executor must be supplied during construction or as an argument to
    /// exec_all().
    void fallback_executor(int16_t, Resource&) {
        LogWarning("SHARED_RESOURCE", "No executor defined");
    }
};
}

#endif
