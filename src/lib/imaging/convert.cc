/// \file convert.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Implementation of the different colorspace converters of
/// Tinkervision.
///
/// This file is part of Tinkervision - Vision Library for Tinkerforge Redbrick
/// \sa https://github.com/Tinkerforge/red-brick
///
/// \copyright
///
/// This program is free software; you can redistribute it and/or
/// modify it under the terms of the GNU General Public License
/// as published by the Free Software Foundation; either version 2
/// of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
/// USA.

#include "convert.hh"

#include <algorithm>
#include <iostream>

tv::Image const& tv::Convert::operator()(tv::Image const& source) {
    operator()(source, target);
    return target;
}

tv::Convert::~Convert(void) {
    if (target.data) {
        delete[] target.data;
    }
}

void tv::Convert::operator()(tv::Image const& source, tv::Image& target) {
    uint16_t width, height;
    size_t bytesize;
    target_format(source.header, width, height, bytesize);

    // Not build for varying ratios of width and height!
    if (not target.data or bytesize != target.header.bytesize) {
        if (target.data) {
            delete[] target.data;
        }
        target.header.width = width;
        target.header.height = height;
        target.header.bytesize = bytesize;
        target.data = new uint8_t[bytesize];
    }

    convert(source, target);
    target.header.timestamp = source.header.timestamp;
    target.header.format = target_format_;
}

tv::ImageHeader tv::Convert::convert_header(ImageHeader const& source) {
    ImageHeader converted;
    target_format(source, converted.width, converted.height,
                  converted.bytesize);

    converted.format = target_format_;
    converted.timestamp = source.timestamp;

    return converted;
}

tv::Convert::Convert(tv::ColorSpace source, tv::ColorSpace target)
    : source_format_{source}, target_format_{target} {}

tv::ConvertYUV422ToYUV420::ConvertYUV422ToYUV420(void)
    : Convert(tv::ColorSpace::YUYV, tv::ColorSpace::YV12) {}
tv::ConvertYUYVToBGR::ConvertYUYVToBGR(void)
    : Convert(tv::ColorSpace::YUYV, tv::ColorSpace::BGR888) {}
tv::ConvertYUYVToRGB::ConvertYUYVToRGB(void)
    : Convert(tv::ColorSpace::YUYV, tv::ColorSpace::RGB888) {}
tv::YV12ToRGBType::YV12ToRGBType(tv::ColorSpace target_format)
    : tv::Convert(tv::ColorSpace::YV12, target_format) {}
tv::ConvertYV12ToRGB::ConvertYV12ToRGB(void)
    : tv::YV12ToRGBType(tv::ColorSpace::RGB888) {}
tv::ConvertYV12ToBGR::ConvertYV12ToBGR(void)
    : tv::YV12ToRGBType(tv::ColorSpace::BGR888) {}
tv::RGBFromToBGR::RGBFromToBGR(ColorSpace from, ColorSpace to)
    : tv::Convert(from, to) {}
tv::ConvertRGBToBGR::ConvertRGBToBGR(void)
    : tv::RGBFromToBGR(tv::ColorSpace::RGB888, tv::ColorSpace::BGR888) {}
tv::ConvertBGRToRGB::ConvertBGRToRGB(void)
    : RGBFromToBGR(tv::ColorSpace::BGR888, tv::ColorSpace::RGB888) {}
tv::ConvertBGRToYV12::ConvertBGRToYV12(void)
    : Convert(tv::ColorSpace::BGR888, tv::ColorSpace::YV12) {}
tv::ConvertBGRToYUYV::ConvertBGRToYUYV(void)
    : Convert(tv::ColorSpace::BGR888, tv::ColorSpace::YUYV) {}
tv::ConvertBGRToGray::ConvertBGRToGray(void)
    : Convert(tv::ColorSpace::BGR888, tv::ColorSpace::GRAY) {}
tv::ConvertGrayToBGR::ConvertGrayToBGR(void)
    : Convert(tv::ColorSpace::GRAY, tv::ColorSpace::BGR888) {}

void tv::ConvertYUV422ToYUV420::target_format(tv::ImageHeader const& source,
                                              uint16_t& target_width,
                                              uint16_t& target_height,
                                              size_t& target_bytesize) const {
    target_width = source.width;
    target_height = source.height;
    target_bytesize = (source.bytesize >> 2) * 3;
}

// output is in order y-block, v-block, u-block
void tv::ConvertYUV422ToYUV420::convert_any(tv::Image const& source,
                                            tv::Image& target, uint8_t* u_ptr,
                                            uint8_t* v_ptr) const {
    auto to = target.data;  // moving pointer

    // Y: every second byte
    for (size_t i = 0; i < source.header.bytesize; i += 2) {
        *to++ = *(source.data + i);
    }

    const size_t width = source.header.width * 2;  // in byte
    const size_t height = source.header.height;

    auto copy_u_or_v = [&width, &height, &to](uint8_t const* u_or_v_ptr) {

        auto next_row = u_or_v_ptr + width;

        for (size_t i = 0; i < height; i += 2) {
            for (size_t j = 0; j < width; j += 4) {
                *to++ = (static_cast<int>(u_or_v_ptr[j]) + next_row[j]) / 2;
            }
            // next two rows
            u_or_v_ptr += (width * 2);
            next_row += (width * 2);
        }
    };

    // U and V: averaging values from two rows a time
    copy_u_or_v(v_ptr);
    copy_u_or_v(u_ptr);
}

void tv::YUVToRGB::target_size(tv::ImageHeader const& source,
                               uint16_t& target_width, uint16_t& target_height,
                               size_t& target_bytesize) const {
    target_width = source.width;
    target_height = source.height;
    target_bytesize = (source.width * source.height) * 3;  // 24 bit/pixel
}

int constexpr tv::YUVToRGB::coeff_r[];
int constexpr tv::YUVToRGB::coeff_g[];
int constexpr tv::YUVToRGB::coeff_b[];

template <size_t r, size_t g, size_t b>
void tv::YUVToRGB::convert(int y, int u, int v, uint8_t* rgb) const {

    // need indices 0,1,2
    static_assert(((r + g + b) == 3) and (r < 3) and (g < 3) and (b < 3) and
                      ((r == 0) or (g == 0) or (b == 0)),
                  "Need to provide rgb channels as 0, 1 and 2");
    y = y - 16;
    u = u - 128;
    v = v - 128;

    // Conversions from http://en.wikipedia.org/wiki/YUV, seem fine.
    // HD:

    *(rgb + r) = clamp_(y + 1.28033 * v);
    *(rgb + g) = clamp_(y - 0.21482 * u - 0.38059 * v);
    *(rgb + b) = clamp_(y + 2.21798 * u);
    // SD:
    /*
    *(rgb + r) = clamp_(y + 1.3983 * v);
    *(rgb + g) = clamp_(y - 0.39465 * u - 0.58060 * v);
    *(rgb + b) = clamp_(y + 2.03211 * u);
    */
    // Kaufmann:
    /*
    *(rgb + r) =
        clamp_((coeff_r[0] * y + coeff_r[1] * u + coeff_r[2] * v) /
    normalizer);

    *(rgb + g) =
        clamp_((coeff_g[0] * y + coeff_g[1] * u + coeff_g[2] * v) /
    normalizer);

    *(rgb + b) =
        clamp_((coeff_b[0] * y + coeff_b[1] * u + coeff_b[2] * v) /
    normalizer);
    */
}

template <size_t r, size_t g, size_t b>
void tv::YUYVToRGBType::convert(tv::Image const& source,
                                tv::Image& target) const {
    assert(source.header.format == ColorSpace::YUYV);

    auto to = target.data;

    for (auto src = source.data;
         src < (source.data + source.header.bytesize);) {

        int const y1 = static_cast<int>(*src++);
        int const u = static_cast<int>(*src++);
        int const y2 = static_cast<int>(*src++);
        int const v = static_cast<int>(*src++);

        YUVToRGB::convert<r, g, b>(y1, u, v, to);
        to += 3;
        YUVToRGB::convert<r, g, b>(y2, u, v, to);
        to += 3;
    }
}

void tv::ConvertYUYVToRGB::target_format(tv::ImageHeader const& source,
                                         uint16_t& target_width,
                                         uint16_t& target_height,
                                         size_t& target_bytesize) const {
    target_size(source, target_width, target_height, target_bytesize);
}

void tv::ConvertYUYVToBGR::target_format(tv::ImageHeader const& source,
                                         uint16_t& target_width,
                                         uint16_t& target_height,
                                         size_t& target_bytesize) const {
    target_size(source, target_width, target_height, target_bytesize);
}

template <uint8_t r, uint8_t g, uint8_t b>
void tv::YV12ToRGBType::convert(tv::Image const& source,
                                tv::Image& target) const {
    assert(source.header.format == ColorSpace::YV12);

    auto to = target.data;
    auto const v_plane =
        source.data + source.header.width * source.header.height;
    auto const u_plane =
        v_plane + ((source.header.width * source.header.height) >> 2);
    auto const uv_offset = source.header.width >> 1;

    // abbreviated version of http://en.wikipedia.org/wiki/YUV
    for (size_t i = 0; i < source.header.height; i++) {
        auto const row_uv = (i >> 1) * uv_offset;
        auto const row_y = i * source.header.width;
        for (size_t j = 0; j < source.header.width; j++) {

            // Every u(v)-value corresponds to one four-block of
            // Y-values,
            // i.e. each two y's from subsequent rows. According to
            // this,
            // the index of the u(v)-value needed is given by:
            // uv_idx = ((i / 2) * (source.width / 2)) + (j / 2)
            // (i.e. like indexing a 2d array with one idx).
            // With the definition of row_uv it can be written:

            auto uv_idx = (j >> 1) + row_uv;

            int const y = static_cast<int>(source.data[row_y + j]);
            int const u = static_cast<int>(u_plane[uv_idx]);
            int const v = static_cast<int>(v_plane[uv_idx]);

            YUVToRGB::convert<r, g, b>(y, u, v, to);
            to += 3;
        }
    }
}

void tv::ConvertYV12ToRGB::target_format(tv::ImageHeader const& source,
                                         uint16_t& target_width,
                                         uint16_t& target_height,
                                         size_t& target_bytesize) const {
    target_size(source, target_width, target_height, target_bytesize);
}

void tv::ConvertYV12ToRGB::convert(tv::Image const& source,
                                   tv::Image& target) const {
    tv::YV12ToRGBType::convert<0, 1, 2>(source, target);
}

void tv::ConvertYV12ToBGR::target_format(tv::ImageHeader const& source,
                                         uint16_t& target_width,
                                         uint16_t& target_height,
                                         size_t& target_bytesize) const {
    target_size(source, target_width, target_height, target_bytesize);
}

void tv::ConvertYV12ToBGR::convert(tv::Image const& source,
                                   tv::Image& target) const {
    tv::YV12ToRGBType::convert<2, 1, 0>(source, target);
}

void tv::RGBFromToBGR::target_size(tv::ImageHeader const& source,
                                   uint16_t& target_width,
                                   uint16_t& target_height,
                                   size_t& target_bytesize) const {
    target_width = source.width;
    target_height = source.height;
    target_bytesize = source.bytesize;
}

void tv::RGBFromToBGR::convert(tv::Image const& source,
                               tv::Image& target) const {
    auto to = target.data;
    for (size_t i = 0; i < source.header.bytesize; i += 3) {
        *to++ = source.data[i + 2];
        *to++ = source.data[i + 1];
        *to++ = source.data[i];
    }
}

void tv::ConvertRGBToBGR::target_format(tv::ImageHeader const& source,
                                        uint16_t& target_width,
                                        uint16_t& target_height,
                                        size_t& target_bytesize) const {

    target_size(source, target_width, target_height, target_bytesize);
}

void tv::ConvertBGRToRGB::target_format(tv::ImageHeader const& source,
                                        uint16_t& target_width,
                                        uint16_t& target_height,
                                        size_t& target_bytesize) const {

    target_size(source, target_width, target_height, target_bytesize);
}

void tv::ConvertBGRToYV12::target_format(tv::ImageHeader const& source,
                                         uint16_t& target_width,
                                         uint16_t& target_height,
                                         size_t& target_bytesize) const {

    target_width = source.width;
    target_height = source.height;
    target_bytesize = (source.width * source.height * 3) >> 1;
}

void tv::ConvertBGRToYV12::convert(Image const& source, Image& target) const {
    assert(source.header.format == ColorSpace::BGR888);

    auto row0 = source.data;
    auto row1 = source.data + source.header.width * 3;
    auto y0 = target.data;
    auto y1 = target.data + target.header.width;
    auto v = target.data + target.header.width * target.header.height;
    auto u = v + ((target.header.width * target.header.height) >> 2);

    for (size_t i = 0; i < source.header.height; i += 2) {
        for (size_t j = 0; j < source.header.width; j += 2) {
            *y0++ = 0.299 * row0[2] + 0.587 * row0[1] + 0.114 * row0[0];
            *y0++ = 0.299 * row0[5] + 0.587 * row0[4] + 0.114 * row0[3];
            *y1++ = 0.299 * row1[2] + 0.587 * row1[1] + 0.114 * row1[0];
            *y1++ = 0.299 * row1[5] + 0.587 * row1[4] + 0.114 * row1[3];
            *u++ = ((0.499 * row0[0] - 0.331 * row0[1] - 0.169 * row0[2]) +
                    (0.499 * row1[0] - 0.331 * row1[1] - 0.169 * row1[2]) +
                    (0.499 * row0[3] - 0.331 * row0[4] - 0.169 * row0[5]) +
                    (0.499 * row1[3] - 0.331 * row1[4] - 0.169 * row1[5])) /
                       4.0 +
                   128;
            *v++ = ((-0.0813 * row0[0] - 0.418 * row0[1] + 0.499 * row0[2]) +
                    (-0.0813 * row1[0] - 0.418 * row1[1] + 0.499 * row1[2]) +
                    (-0.0813 * row0[3] - 0.418 * row0[4] + 0.499 * row0[5]) +
                    (-0.0813 * row1[3] - 0.418 * row1[4] + 0.499 * row1[5])) /
                       4.0 +
                   128;

            row0 += 6;
            row1 += 6;
        }

        y0 = y1;
        y1 = y1 + target.header.width;
        row0 = row1;
        row1 += source.header.width * 3;
    }
}

void tv::ConvertBGRToYUYV::target_format(tv::ImageHeader const& source,
                                         uint16_t& target_width,
                                         uint16_t& target_height,
                                         size_t& target_bytesize) const {

    target_width = source.width;
    target_height = source.height;
    target_bytesize = (source.width * source.height * 3) >> 1;
}

void tv::ConvertBGRToYUYV::convert(Image const& source, Image& target) const {
    assert(source.header.format == ColorSpace::BGR888);

    auto rgb = source.data;
    auto y0 = target.data;
    auto u = y0 + 1;
    auto y1 = u + 1;
    auto v = y1 + 1;

    for (size_t i = 0; i < source.header.height; ++i) {
        for (size_t j = 0; j < source.header.width; j += 2) {

            *y0 = 0.299 * rgb[2] + 0.587 * rgb[1] + 0.114 * rgb[0];
            *y1 = 0.299 * rgb[5] + 0.587 * rgb[4] + 0.114 * rgb[3];
            *u = ((0.499 * rgb[0] - 0.331 * rgb[1] - 0.169 * rgb[2]) +
                  (0.499 * rgb[3] - 0.331 * rgb[4] - 0.169 * rgb[5])) /
                     2.0 +
                 128;
            *v = ((-0.0813 * rgb[0] - 0.418 * rgb[1] + 0.499 * rgb[2]) +
                  (-0.0813 * rgb[3] - 0.418 * rgb[4] + 0.499 * rgb[5])) /
                     2.0 +
                 128;

            rgb += 6;
        }

        y0 += 4;
        y1 += 4;
        u += 4;
        v += 4;
    }
}

void tv::ConvertBGRToGray::target_format(tv::ImageHeader const& source,
                                         uint16_t& target_width,
                                         uint16_t& target_height,
                                         size_t& target_bytesize) const {
    target_width = source.width;
    target_height = source.height;
    target_bytesize = source.bytesize / 3;
}

void tv::ConvertBGRToGray::convert(Image const& source, Image& target) const {
    assert(source.header.format == ColorSpace::BGR888);

    auto bgr = source.data;
    auto gray = target.data;
    for (size_t i = 0; i < target.header.bytesize; ++i) {
        *gray = static_cast<uint8_t>(
            std::round(0.114 * bgr[2] + 0.587 * bgr[1] + 0.299 * bgr[0]));
        // above formula yields 255 for b=g=r=255, no clamping needed
        bgr += 3;
        ++gray;
    }
}

void tv::ConvertGrayToBGR::target_format(tv::ImageHeader const& source,
                                         uint16_t& target_width,
                                         uint16_t& target_height,
                                         size_t& target_bytesize) const {
    target_width = source.width;
    target_height = source.height;
    target_bytesize = source.bytesize * 3;
}

void tv::ConvertGrayToBGR::convert(Image const& source, Image& target) const {
    assert(source.header.format == ColorSpace::GRAY);

    auto bgr = target.data;
    auto gray = source.data;
    for (size_t i = 0; i < source.header.bytesize; ++i) {
        bgr[0] = bgr[1] = bgr[2] = *gray;
        bgr += 3;
        ++gray;
    }
}

tv::Converter::Converter(tv::ColorSpace source, tv::ColorSpace target) {
    auto it = std::find_if(conversions_.begin(), conversions_.end(),
                           [&source, &target](Conversion const& conversion) {

        return (std::get<0>(conversion) == source) and
               (std::get<1>(conversion) == target);
    });

    if (it != conversions_.end()) {
        auto maker = std::get<2>(*it);
        converter_ = (this->*maker)();
    }
}

tv::Converter::~Converter(void) {
    if (converter_) {
        delete converter_;
    }
}

tv::Image const& tv::Converter::operator()(
    tv::ImageAllocator const& source) const {
    if (not converter_) {
        return invalid_image_;
    }

    return (*converter_)(source());
}

tv::Image const& tv::Converter::operator()(tv::Image const& source) const {
    if (not converter_) {
        return invalid_image_;
    }

    return (*converter_)(source);
}

void tv::Converter::operator()(tv::Image const& source,
                               tv::Image& target) const {
    if (converter_) {
        (*converter_)(source, target);
    }
}

tv::ImageHeader tv::Converter::convert_header(ImageHeader const& source) const {
    if (not converter_) {
        return ImageHeader();
    }

    return converter_->convert_header(source);
}
