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

bool tinkervision::Colortracking::valid(void) const {
    return callback_ and(min_hue_ < max_hue_);
}
