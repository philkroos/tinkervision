#include "module.hh"

void tv::Module::execute(tv::Image const& image) { tv_module_->execute(image); }

tv::ColorSpace tv::Module::expected_format(void) const {
    return tv_module_->expected_format();
}

std::string tv::Module::name(void) const { return tv_module_->name(); }
tv::ModuleType const& tv::Module::type(void) const {
    return tv_module_->type();
}

void tv::Module::get_parameters_list(
    std::vector<std::string>& parameters) const {
    tv_module_->parameter_list(parameters);
}

bool tv::Module::has_parameter(std::string const& parameter) const {
    return tv_module_->has_parameter(parameter);
}

bool tv::Module::set(std::string const& parameter, TV_Word value) {
    return tv_module_->set(parameter, value);
}
TV_Word tv::Module::get(std::string const& parameter) {
    return tv_module_->get(parameter);
}

tv::Result const* tv::Module::result(void) const {
    return tv_module_->get_result();
}

bool tv::Module::running(void) const noexcept { return tv_module_->running(); }
