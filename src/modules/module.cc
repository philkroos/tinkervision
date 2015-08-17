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
