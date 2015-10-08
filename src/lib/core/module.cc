#include "module.hh"

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

void tfv::Module::get_parameters_list(
    std::vector<std::string>& parameters) const {
    tv_module_->parameter_list(parameters);
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

tfv::Result const* tfv::Module::result(void) const {
    return tv_module_->get_result();
}

bool tfv::Module::running(void) const noexcept { return tv_module_->running(); }
