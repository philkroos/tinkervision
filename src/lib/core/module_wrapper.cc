#include "module_wrapper.hh"

void tv::ModuleWrapper::execute(tv::Image const& image) {
    static decltype(period_) current{0};

    /// Execute the module if period_ is not 0 and the module is scheduled to
    /// run in this cycle, decided by comparing the value of period_ with an
    /// internal counter.
    if (period_ and ++current >= period_) {
        current = 0;
        tv_module_->execute(image);
    }
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

    /// Also appended are the parameters valid for all modules:
    ///   - \c "period": Sets the period_ value.
    parameters.push_back("period");
}

bool tv::ModuleWrapper::has_parameter(std::string const& parameter) const {
    /// Some parameters are supported by all modules.
    /// \see get_parameters_list().
    return parameter == "period" or tv_module_->has_parameter(parameter);
}

bool tv::ModuleWrapper::set_parameter(std::string const& parameter,
                                      TV_Word value) {
    if (parameter == "period") {
        if (value >= 0 and value <= std::numeric_limits<uint8_t>::max()) {
            period_ = static_cast<decltype(period_)>(value);
            return true;
        }
        return false;
    }
    return tv_module_->set(parameter, value);
}

TV_Word tv::ModuleWrapper::get_parameter(std::string const& parameter) {
    if (parameter == "period") {
        return static_cast<TV_Word>(period_);
    }
    return tv_module_->get(parameter);
}

tv::Result const* tv::ModuleWrapper::result(void) const {
    return tv_module_->get_result();
}

bool tv::ModuleWrapper::running(void) const noexcept {
    return tv_module_->running();
}
