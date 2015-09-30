#include "module.hh"

bool tfv::is_compatible_callback(tfv::Result const* result,
                                 TFV_CallbackPoint const&) {
    return dynamic_cast<tfv::PointResult const*>(result) != nullptr;
}

bool tfv::is_compatible_callback(tfv::Result const* result,
                                 TFV_CallbackValue const&) {
    return dynamic_cast<tfv::ScalarResult const*>(result) != nullptr;
}

bool tfv::is_compatible_callback(tfv::Result const* result,
                                 TFV_CallbackString const&) {
    return dynamic_cast<tfv::StringResult const*>(result) != nullptr;
}

bool tfv::is_compatible_callback(tfv::Result const* result,
                                 TFV_CallbackRectangle const&) {
    return dynamic_cast<tfv::RectangleResult const*>(result) != nullptr;
}

template <>
tfv::Callback* tfv::make_callback<TFV_CallbackPoint>(
    TFV_Int id, TFV_CallbackPoint cb_func) {
    return new tfv::PointCallback(id, cb_func);
}

template <>
tfv::Callback* tfv::make_callback<TFV_CallbackValue>(
    TFV_Int id, TFV_CallbackValue cb_func) {
    return new tfv::ValueCallback(id, cb_func);
}

template <>
tfv::Callback* tfv::make_callback<TFV_CallbackRectangle>(
    TFV_Int id, TFV_CallbackRectangle cb_func) {
    return new tfv::RectangleCallback(id, cb_func);
}

template <>
tfv::Callback* tfv::make_callback<TFV_CallbackString>(
    TFV_Int id, TFV_CallbackString cb_func) {
    return new tfv::StringCallback(id, cb_func);
}

void tfv::ValueCallback::operator()(tfv::Result const* result) {
    cb(id, dynamic_cast<ScalarResult const*>(result)->scalar, nullptr);
}

void tfv::PointCallback::operator()(tfv::Result const* result) {
    auto point = dynamic_cast<PointResult const*>(result);
    cb(id, point->x, point->y, nullptr);
}

void tfv::RectangleCallback::operator()(tfv::Result const* result) {
    auto rect = dynamic_cast<RectangleResult const*>(result);
    cb(id, rect->x, rect->y, rect->width, rect->height, nullptr);
}

void tfv::StringCallback::operator()(tfv::Result const* result) {
    cb(id, dynamic_cast<StringResult const*>(result)->result.c_str(), nullptr);
}

void tfv::Module::execute(tfv::Image const& image) {
    tv_module_->execute(image);
}

tfv::ColorSpace tfv::Module::expected_format(void) const {
    return tv_module_->expected_format();
}

std::string tfv::Module::name(void) const { return tv_module_->name(); }
tfv::ModuleType const& tfv::Module::type(void) const {
    return tv_module_->type();
}

bool tfv::Module::has_parameter(std::string const& parameter) const {
    return tv_module_->has_parameter(parameter);
}

bool tfv::Module::set(std::string const& parameter, TFV_Word value) {
    return tv_module_->set(parameter, value);
}
TFV_Word tfv::Module::get(std::string const& parameter) {
    return tv_module_->get(parameter);
}

tfv::Result const* tfv::Module::get_result(void) const {
    return tv_module_->get_result();
}

bool tfv::Module::running(void) const noexcept { return tv_module_->running(); }
