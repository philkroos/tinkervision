#ifndef WINDOW_H
#define WINDOW_H

#ifdef DEV

#include <iostream>

#include <opencv2/opencv.hpp>

#include "tinkervision_defines.h"

namespace tfv {
class Window {
private:
    std::string name;

public:
    explicit Window(std::string name) : name(name) { cv::namedWindow(name); }

    void update(TFV_ImageData* data, int rows, int columns) {
        cv::Mat frame(rows, columns, CV_8UC3, data);
        cv::imshow(name, frame);
        cv::waitKey(100);  // skip time to give window-update thread a chance
    }
};
}

#endif  // DEV

#endif /* WINDOW_H */
