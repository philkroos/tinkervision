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

#ifndef MODULE_H
#define MODULE_H

#include <typeinfo>
#include <type_traits>
#include <cassert>

#include "tinkervision_defines.h"
#include "image.hh"
#include "bitflag.hh"
#include "logger.hh"
#include "tv_module.hh"

namespace tfv {

class Result;
class TVModule;

bool is_compatible_callback(Result const* result, TFV_CallbackPoint const&);
bool is_compatible_callback(Result const* result, TFV_CallbackValue const&);
bool is_compatible_callback(Result const* result, TFV_CallbackRectangle const&);
bool is_compatible_callback(Result const* result, TFV_CallbackString const&);

struct Callback {
    TFV_Int id{};
    virtual ~Callback(void) = default;
    explicit Callback(TFV_Int id) : id(id) {}
    virtual void operator()(Result const* result) = 0;
};

struct PointCallback : public Callback {
    virtual ~PointCallback(void) = default;

    TFV_CallbackPoint cb;
    PointCallback(TFV_Int id, TFV_CallbackPoint cb) : Callback(id), cb(cb) {}
    void operator()(Result const* result) override final;
};

struct ValueCallback : public Callback {
    virtual ~ValueCallback(void) = default;

    TFV_CallbackValue cb;
    ValueCallback(TFV_Int id, TFV_CallbackValue cb) : Callback(id), cb(cb) {}
    void operator()(Result const* result) override final;
};

struct RectangleCallback : public Callback {
    virtual ~RectangleCallback(void) = default;

    TFV_CallbackRectangle cb;
    RectangleCallback(TFV_Int id, TFV_CallbackRectangle cb)
        : Callback(id), cb(cb) {}
    void operator()(Result const* result) override final;
};

struct StringCallback : public Callback {
    virtual ~StringCallback(void) = default;

    TFV_CallbackString cb;
    StringCallback(TFV_Int id, TFV_CallbackString cb) : Callback(id), cb(cb) {}
    void operator()(Result const* result) override final;
};

template <typename Func>
Callback* make_callback(TFV_Int id, Func cb_func);

template <>
Callback* make_callback<TFV_CallbackPoint>(TFV_Int id,
                                           TFV_CallbackPoint cb_func);

template <>
Callback* make_callback<TFV_CallbackValue>(TFV_Int id,
                                           TFV_CallbackValue cb_func);

template <>
Callback* make_callback<TFV_CallbackRectangle>(TFV_Int id,
                                               TFV_CallbackRectangle cb_func);

template <>
Callback* make_callback<TFV_CallbackString>(TFV_Int id,
                                            TFV_CallbackString cb_func);

struct CallbackWrapper {
    ~CallbackWrapper(void) {
        if (cb) {
            delete cb;
        }
    }
    Callback* cb = nullptr;
    template <typename Func>
    void make(TFV_Int id, Func callback_function) {
        cb = make_callback(id, callback_function);
    }
};

class Module {
public:
    enum class Tag : unsigned {
        // static tags
        None = 0x00,
        TVModule = 0x01,
        Fx = 0x02,
        Analysis = 0x04,
        Output = 0x08,

        // runtime tags
        ExecAndRemove = 0x10,
        ExecAndDisable = 0x20,
        Removable = 0x30,
        Sequential = 0x40
    };

private:
    bool active_;
    Module::Tag tags_;
    TFV_Int module_id_;

    TVModule* tv_module_;

    CallbackWrapper cb_;

public:
    // deprecated
    Module(TFV_Int module_id, std::string type, Tag tags)
        : active_{true},
          tags_{tags},
          module_id_{module_id},
          tv_module_{nullptr} {
        Log("MODULE", "Constructing module ", module_id);
    }

    // deprecated
    Module(TFV_Int module_id, std::string type)
        : Module(module_id, type, Tag::None) {}

    Module(TVModule* executable, TFV_Int module_id, Tag tags)
        : active_{false},
          tags_(tags),
          module_id_(module_id),
          tv_module_(executable) {
        assert(tv_module_ != nullptr);
    }

    virtual ~Module(void) { Log("MODULE::Destructor", name()); }

    // No copy allowed
    Module(Module const& other) = delete;
    Module(Module&& other) = delete;
    Module& operator=(Module const& rhs) = delete;
    Module& operator=(Module&& rhs) = delete;

    template <typename... Args>
    bool register_callback(void (*callback)(Args...)) {
        auto result = get_result();
        if (not result or not is_compatible_callback(result, callback)) {
            return false;
        }

        cb_.make(module_id_, callback);
        Log("MODULE", "Set callback for ", name(), " (ID ", module_id_, ")");
        return true;
    }

    virtual void execute(tfv::Image const& image);
    virtual void execute_modifying(tfv::Image& image);
    virtual bool modifies_image(void) const;

    TFV_Int id(void) const { return module_id_; }
    std::string name(void) const;

    bool running(void) const noexcept;

    bool enabled(void) const noexcept { return active_; }

    // return false if previous state was the same
    bool enable(void) noexcept { return switch_active(true); }

    // return false if previous state was the same
    bool disable(void) noexcept { return switch_active(false); }

    // return false if previous state was the same
    bool switch_active(bool to_state) {
        Log("MODULE", to_state ? "Enabling " : "Disabling ", "module ",
            module_id_, " (was ", active_ ? "active)" : "inactive)");
        auto was_active = active_;
        active_ = to_state;
        return not(was_active == active_);
    }

    void exec(tfv::Image& image) {
        if (modifies_image()) {
            execute_modifying(image);
        } else {
            execute(image);
        }

        if (cb_.cb) {
            (*cb_.cb)(get_result());
        }
    }
    virtual ColorSpace expected_format(void) const;

    virtual bool has_parameter(std::string const& parameter) const;

    virtual bool set(std::string const& parameter, TFV_Word value);

    virtual TFV_Word get(std::string const& parameter);

    bool set_parameter(std::string const& parameter, TFV_Word value) {
        return set(parameter, value);
    }

    void get_parameter(std::string const& parameter, TFV_Word& value) {
        value = get(parameter);
    }

    virtual Result const* get_result(void) const;

    Tag const& tags(void) const { return tags_; }
    void tag(Tag tags) { tags_ |= tags; }

    TVModule* executable(void) { return tv_module_; }
};
}
#endif
