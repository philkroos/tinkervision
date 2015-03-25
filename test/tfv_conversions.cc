#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>

#include "tinkervision_defines.h"
#include "image.hh"
#include "convert.hh"
#include "testwindow.hh"

void print_max_and_min_yuv(TFV_ImageData* data, size_t width, size_t height) {

    auto y = data;
    auto u = y + 1;
    auto y2 = u + 1;
    auto v = y2 + 1;
    int max_y = 0;
    int min_y = 255;
    int max_u = 0;
    int min_u = 255;
    int max_v = 0;
    int min_v = 255;
    for (size_t i = 0; i < width * height / 2; i++) {
        if (*y > max_y) {
            max_y = *y;
        } else if (*y < min_y) {
            min_y = *y;
        }
        if (*y2 > max_y) {
            max_y = *y2;
        } else if (*y2 < min_y) {
            min_y = *y2;
        }
        if (*u > max_u) {
            max_u = *u;
        } else if (*u < min_u) {
            min_u = *u;
        }
        if (*v > max_v) {
            max_v = *v;
        } else if (*v < min_v) {
            min_v = *v;
        }
        y += 4;
        y2 += 4;
        u += 4;
        v += 4;
    }

    std::cout << "Max y,v,u: " << max_y << "," << max_v << "," << max_u
              << std::endl;
    std::cout << "Min y,v,u: " << min_y << "," << min_v << "," << min_u
              << std::endl;
}

int main() {
    // This was recorded with `luvcview -d /dev/video1 -f YUYV -s 1280x720 -C`
    std::ifstream yuyv("frame.raw",
                       std::ios::in | std::ios::binary | std::ios::ate);

    tfv::Image original(tfv::ImageFormat::YUYV);

    auto width = 1280;
    auto height = 720;

    original.width = width;
    original.height = height;
    original.bytesize = original.width * original.height * 2;

    if (yuyv.is_open() and size_t(yuyv.tellg()) == original.bytesize) {

        original.data = new unsigned char[original.bytesize];
        yuyv.seekg(0, std::ios::beg);
        yuyv.read((char*)original.data, original.bytesize);
        yuyv.close();
        std::cout << "file is in memory: " << original.bytesize << " byte."
                  << std::endl;
    } else {
        return -1;
    }

    print_max_and_min_yuv(original.data, original.width, original.height);

    tfv::Window window;
    TFV_Id win_id = 1;

    tfv::ConvertYUYVToYV12 yv12_converter;
    tfv::ConvertYUYVToRGB rgb1_converter;
    tfv::ConvertYV12ToRGB rgb2_converter;
    tfv::ConvertBGRToRGB rgb3_converter;
    tfv::ConvertYUYVToBGR bgr1_converter;
    tfv::ConvertYV12ToBGR bgr2_converter;
    tfv::ConvertRGBToBGR bgr3_converter;
    auto& yv12_result = yv12_converter(original);

    auto& rgb1_result = rgb1_converter(original);
    auto& rgb2_result = rgb2_converter(yv12_result);

    auto& bgr1_result = bgr1_converter(original);
    auto& bgr2_result = bgr2_converter(yv12_result);

    auto& rgb3_result = rgb3_converter(bgr1_result);
    auto& bgr3_result = bgr3_converter(rgb1_result);

    auto& rgb4_result = rgb3_converter(bgr3_result);

    // File is in format YV12 (420p)
    std::ofstream ofs("/tmp/uvoutput.yuv", std::ios::out | std::ios::binary);
    ofs.write((const char*)yv12_result.data, yv12_result.bytesize);
    ofs.close();

    window.update(win_id, rgb1_result.data, height, width, "YUYV to RGB");
    window.wait_for_input();

    window.update(win_id, rgb2_result.data, "YV12 to RGB");
    window.wait_for_input();

    window.update(win_id, bgr1_result.data, "YUYV to BGR");
    window.wait_for_input();

    window.update(win_id, bgr2_result.data, "YV12 to BGR");
    window.wait_for_input();

    window.update(win_id, rgb3_result.data, "BGR to RGB");
    window.wait_for_input();

    window.update(win_id, bgr3_result.data, "RGB to BGR");
    window.wait_for_input();

    window.update(win_id, rgb4_result.data, "RGB to BGR to RGB");
    window.wait_for_input();

    delete[] original.data;
}
