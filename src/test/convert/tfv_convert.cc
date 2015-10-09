#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>

#include "tinkervision_defines.h"
#include "image.hh"
#include "convert.hh"
#include "testwindow.hh"

void print_max_and_min_yuv(TV_ImageData* data, size_t width, size_t height) {

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

// This program uses the different converters to translate a single 'raw' image
// between the supported colorspaces. Each result is shown in an Opencv-Window.
// NOTE that the "* to BGR" output shows the correct image; the "* to RGB"
// output
// is shown with B and R channels exchanged because OCV's native format is BGR
// and so the viewer interprets RGB images as BGR. However, it serves to verify
// pixel correctness.
int main() {

    // This was recorded with `luvcview -d /dev/video1 -f YUYV -s 1280x720 -C`
    // http://rawpixels.net/ Settings:
    // - Predefined format: YUY2
    // - Pixel format: YVU
    // - Deselect 'Alpha first'
    // - width/height: 1280/720
    // - Pixel Plane: PackedYUV
    std::ifstream yuyv("../frame.raw",
                       std::ios::in | std::ios::binary | std::ios::ate);

    auto width = 1280;
    auto height = 720;

    tv::ImageAllocator original;

    auto bytesize = size_t(width * height * 2);

    if (yuyv.is_open() and size_t(yuyv.tellg()) == bytesize) {

        // data managed by image
        original.allocate(width, height, bytesize, tv::ColorSpace::YUYV, false);
        yuyv.seekg(0, std::ios::beg);
        yuyv.read((char*)original.image().data, bytesize);
        yuyv.close();
        std::cout << "file is in memory: " << original.image().header.bytesize
                  << " byte." << std::endl;
    } else {
        std::cout << "Input file frame.raw not found" << std::endl;
        return -1;
    }

    auto& header = original.image().header;
    auto& image = original.image();
    print_max_and_min_yuv(image.data, header.width, header.height);

    tv::Window window;
    TV_Id win_id = 1;

    auto yuyvToYv12_converter =
        tv::Converter(tv::ColorSpace::YUYV, tv::ColorSpace::YV12);
    auto yuyvToRgb_converter =
        tv::Converter(tv::ColorSpace::YUYV, tv::ColorSpace::RGB888);
    auto yv12ToRgb_converter =
        tv::Converter(tv::ColorSpace::YV12, tv::ColorSpace::RGB888);
    auto bgrToRgb_converter =
        tv::Converter(tv::ColorSpace::BGR888, tv::ColorSpace::RGB888);
    auto yuyvToBgr_converter =
        tv::Converter(tv::ColorSpace::YUYV, tv::ColorSpace::BGR888);
    auto yv12ToBgr_converter =
        tv::Converter(tv::ColorSpace::YV12, tv::ColorSpace::BGR888);
    auto rgbToBgr_converter =
        tv::Converter(tv::ColorSpace::RGB888, tv::ColorSpace::BGR888);
    auto bgrToYv12_converter =
        tv::Converter(tv::ColorSpace::BGR888, tv::ColorSpace::YV12);
    auto bgrToGray_converter =
        tv::Converter(tv::ColorSpace::BGR888, tv::ColorSpace::GRAY);
    auto grayToBgr_converter =
        tv::Converter(tv::ColorSpace::GRAY, tv::ColorSpace::BGR888);

    // operator() returns tv::Image
    auto& yuyvToYv12_result = yuyvToYv12_converter(original);
    auto& yuyvToRgb_result = yuyvToRgb_converter(original);
    auto& yv12ToRgb_result = yv12ToRgb_converter(yuyvToYv12_result);
    auto& yuyvToBgr_result = yuyvToBgr_converter(original);
    auto& yv12ToBgr_result = yv12ToBgr_converter(yuyvToYv12_result);
    auto& bgrToRgb_result = bgrToRgb_converter(yuyvToBgr_result);
    auto& rgbToBgr_result = rgbToBgr_converter(yuyvToRgb_result);
    auto& rgbToBgrToRgb_result = bgrToRgb_converter(rgbToBgr_result);
    auto& bgrToYv12_result = bgrToYv12_converter(yuyvToBgr_result);
    auto& bgrToGray_result = bgrToGray_converter(yuyvToBgr_result);
    auto& grayToBgr_result = grayToBgr_converter(bgrToGray_result);

    // First two are not displayable by opencv, saved as file.
    // Files are in format YV12 (420p). Can be checked here:
    // http://rawpixels.net/ Settings:
    // - Predefined format: YUV420p
    // - Pixel format: YVU
    // - Deselect 'Alpha first'
    // - width/height: 1280/720
    // - Pixel Plane: planar
    std::ofstream ofs("/tmp/uvoutput.yuv", std::ios::out | std::ios::binary);
    ofs.write((const char*)yuyvToYv12_result.data,
              yuyvToYv12_result.header.bytesize);
    ofs.close();
    std::ofstream ofs2("/tmp/uvoutput_after.yuv",
                       std::ios::out | std::ios::binary);
    ofs2.write((const char*)bgrToYv12_result.data,
               bgrToYv12_result.header.bytesize);
    ofs2.close();

    // Next few are viewable with opencv
    window.update(win_id, yuyvToRgb_result.data, height, width, "YUYV to RGB");
    window.wait_for_input();

    window.update(win_id, yv12ToRgb_result.data, "YV12 to RGB");
    window.wait_for_input();

    window.update(win_id, yuyvToBgr_result.data, "YUYV to BGR");
    window.wait_for_input();

    window.update(win_id, yv12ToBgr_result.data, "YV12 to BGR");
    window.wait_for_input();

    window.update(win_id, bgrToRgb_result.data, "BGR to RGB");
    window.wait_for_input();

    window.update(win_id, rgbToBgr_result.data, "RGB to BGR");
    window.wait_for_input();

    window.update(win_id, rgbToBgrToRgb_result.data, "RGB to BGR to RGB");
    window.wait_for_input();

    window.update(win_id, grayToBgr_result.data, "BGR to Gray to BGR");
    window.wait_for_input();

    // second window
    window.update(win_id + 1, bgrToGray_result.data,
                  bgrToGray_result.header.height, bgrToGray_result.header.width,
                  "BGR to Gray", CV_8UC1);
    window.wait_for_input();
}
