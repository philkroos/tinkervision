#include "colortracking.hh"

#ifdef DEV
#include <iostream>
#endif

void tinkervision::Colortracking::execute(TFV_ImageData* data, TFV_Int rows,
                                          TFV_Int columns) {
    // #ifdef DEV
    //     std::cout << "Executing colortracking with cam,min,max: " <<
    // camera_id()
    //               << "," << std::to_string(min_hue_) << ","
    //               << std::to_string(max_hue_) << std::endl;
    // #endif
}

template <>
bool tinkervision::valid<tinkervision::Colortracking>(TFV_Byte& min_hue,
                                                      TFV_Byte& max_hue,
                                                      TFV_Callback& callback,
                                                      TFV_Context& context) {
#ifdef DEV
    std::cout << "Colortracking::valid" << std::endl;
#endif  // DEV
    return min_hue < max_hue and callback;
}

template <>
void tinkervision::set<tinkervision::Colortracking>(
    tinkervision::Colortracking* ct, TFV_Byte min_hue, TFV_Byte max_hue,
    TFV_Callback callback, TFV_Context context) {

#ifdef DEV
    std::cout << "Colortracking::set" << std::endl;
#endif  // DEV
    ct->min_hue_ = min_hue;
    ct->max_hue_ = max_hue;
    ct->callback_ = callback;
    ct->context_ = context;
}
