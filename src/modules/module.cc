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
    tv_module_->execute(image);
}
void tfv::Module::execute_modifying(tfv::Image& image) {
    tv_module_->execute_modifying(image);
}
bool tfv::Module::modifies_image(void) const {
    return tv_module_->modifies_image();
}

tfv::ColorSpace tfv::Module::expected_format(void) const {
    return tv_module_->expected_format();
}

std::string tfv::Module::name(void) const { return tv_module_->name(); }

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
