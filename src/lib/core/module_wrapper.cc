#include "module_wrapper.hh"

void tv::ModuleWrapper::execute(tv::Image const& image) {
    tv_module_->execute(image);
}

tv::ColorSpace tv::ModuleWrapper::expected_format(void) const {
    return tv_module_->expected_format();
}

std::string tv::ModuleWrapper::name(void) const { return tv_module_->name(); }
tv::ModuleType const& tv::ModuleWrapper::type(void) const {
    return tv_module_->type();
}

void tv::ModuleWrapper::get_parameters_list(
    std::vector<std::string>& parameters) const {
    tv_module_->parameter_list(parameters);
}

bool tv::ModuleWrapper::has_parameter(std::string const& parameter) const {
    return tv_module_->has_parameter(parameter);
}

bool tv::ModuleWrapper::set(std::string const& parameter, TV_Word value) {
    return tv_module_->set(parameter, value);
}
TV_Word tv::ModuleWrapper::get(std::string const& parameter) {
    return tv_module_->get(parameter);
}

tv::Result const* tv::ModuleWrapper::result(void) const {
    return tv_module_->get_result();
}

bool tv::ModuleWrapper::running(void) const noexcept {
    return tv_module_->running();
}
