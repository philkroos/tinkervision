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

struct Result;
class TVModule;
enum class ModuleType : uint8_t;

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
    ~PointCallback(void) override final = default;

    TFV_CallbackPoint cb;
    PointCallback(TFV_Int id, TFV_CallbackPoint cb) : Callback(id), cb(cb) {}
    void operator()(Result const* result) override final;
};

struct ValueCallback : public Callback {
    ~ValueCallback(void) override final = default;

    TFV_CallbackValue cb;
    ValueCallback(TFV_Int id, TFV_CallbackValue cb) : Callback(id), cb(cb) {}
    void operator()(Result const* result) override final;
};

struct RectangleCallback : public Callback {
    ~RectangleCallback(void) override final = default;

    TFV_CallbackRectangle cb;
    RectangleCallback(TFV_Int id, TFV_CallbackRectangle cb)
        : Callback(id), cb(cb) {}
    void operator()(Result const* result) override final;
};

struct StringCallback : public Callback {
    ~StringCallback(void) override final = default;

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
    // runtime tags describing some sort of status this module is in, as
    // relevant for the execution of the Api
    enum class Tag : unsigned {
        None = 0x01,
        ExecAndRemove = 0x02,
        ExecAndDisable = 0x04,
        Removable = 0x08,
        Sequential = 0x10,
    };

private:
    bool active_;
    Module::Tag tags_ = Module::Tag::None;
    TFV_Int module_id_;

    TVModule* tv_module_;

    CallbackWrapper cb_;

public:
    Module(TVModule* executable, TFV_Int module_id)
        : active_(false), module_id_(module_id), tv_module_(executable) {}

    ~Module(void) { Log("MODULE::Destructor", name()); }

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

    void execute(tfv::Image const& image);
    void execute_modifying(tfv::Image& image);

    TFV_Int id(void) const { return module_id_; }
    std::string name(void) const;
    ModuleType const& type(void) const;

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
        execute(image);
        if (cb_.cb) {
            (*cb_.cb)(get_result());
        }
    }

    ColorSpace expected_format(void) const;

    bool has_parameter(std::string const& parameter) const;

    bool set(std::string const& parameter, TFV_Word value);

    TFV_Word get(std::string const& parameter);

    bool set_parameter(std::string const& parameter, TFV_Word value) {
        return set(parameter, value);
    }

    void get_parameter(std::string const& parameter, TFV_Word& value) {
        value = get(parameter);
    }

    Result const* get_result(void) const;

    Tag const& tags(void) const { return tags_; }
    void tag(Tag tags) { tags_ |= tags; }

    TVModule* executable(void) { return tv_module_; }
};
}
#endif
