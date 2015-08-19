#include "module.hh"

bool tfv::is_compatible_callback(tfv::Result const* result,
                                 TFV_CallbackPoint const&) {
    return dynamic_cast<tfv::PointResult const*>(result) != nullptr;
};

bool tfv::is_compatible_callback(tfv::Result const* result,
                                 TFV_CallbackValue const&) {
    return dynamic_cast<tfv::ScalarResult const*>(result) != nullptr;
};

bool tfv::is_compatible_callback(tfv::Result const* result,
                                 TFV_CallbackString const&) {
    return dynamic_cast<tfv::StringResult const*>(result) != nullptr;
};

bool tfv::is_compatible_callback(tfv::Result const* result,
                                 TFV_CallbackRectangle const&) {
    return dynamic_cast<tfv::RectangleResult const*>(result) != nullptr;
};

void tfv::Module::execute(tfv::Image const& image) {
    executable_->execute(image);
}
void tfv::Module::execute_modifying(tfv::Image& image) {
    executable_->execute_modifying(image);
}
bool tfv::Module::modifies_image(void) const {
    return executable_->modifies_image();
}

tfv::ColorSpace tfv::Module::expected_format(void) const {
    return executable_->expected_format();
}

bool tfv::Module::has_parameter(std::string const& parameter) const {
    return executable_->has_parameter(parameter);
}

bool tfv::Module::set(std::string const& parameter, TFV_Word value) {
    return executable_->set(parameter, value);
}
TFV_Word tfv::Module::get(std::string const& parameter) {
    return executable_->get(parameter);
}

tfv::Result const* tfv::Module::get_result(void) const {
    return executable_->get_result();
}

tfv::Module::Tag const& tfv::Module::tags(void) const {
    return executable_->tags();
}
void tfv::Module::tag(tfv::Module::Tag tags) { executable_->tag(tags); }

bool tfv::Module::running(void) const noexcept {
    return executable_->running();
}
