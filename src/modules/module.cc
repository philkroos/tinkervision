#include "module.hh"

bool tfv::is_compatible_callback(tfv::Result const& result,
                                 TFV_CallbackPoint const&) {
    return typeid(result) == typeid(tfv::PointResult);
};

bool tfv::is_compatible_callback(tfv::Result const& result,
                                 TFV_CallbackValue const&) {
    return typeid(result) == typeid(tfv::ScalarResult);
};
