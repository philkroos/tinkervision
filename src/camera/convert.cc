/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014-2015 philipp.kroos@fh-bielefeld.de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "convert.hh"

#include <algorithm>

tfv::ConvertRGBToBGR::ConvertRGBToBGR(void) {}
tfv::ConvertRGBToBGR::~ConvertRGBToBGR(void) {}

tfv::Convert::~Convert(void) {
    if (target) {
        delete target;
    }
}

tfv::Image const& tfv::Convert::operator()(tfv::Image const& source) {
    if (not target) {
        size_t width, height, bytesize;
        target_format_ = target_format(source, width, height, bytesize);
        target = new tfv::Image();
        target->width = width;
        target->height = height;
        target->bytesize = bytesize;
        target->format = target_format_;
        target->data = new TFV_ImageData[bytesize];
    }
    convert(source, *target);
    return *target;
}

tfv::ImageFormat tfv::ConvertYUV422ToYUV420::target_format(
    tfv::Image const& source, size_t& target_width, size_t& target_height,
    size_t& target_bytesize) const {
    target_width = source.width;
    target_height = source.height;
    target_bytesize = (source.bytesize >> 2) * 3;
    return tfv::ImageFormat::YV12;
}

// output is in order y-block, v-block, u-block
void tfv::ConvertYUV422ToYUV420::convert_any(tfv::Image const& source,
                                             tfv::Image& target,
                                             TFV_ImageData* u_ptr,
                                             TFV_ImageData* v_ptr) const {

    auto to = target.data;  // moving pointer

    // Y: every second byte
    for (size_t i = 0; i < source.bytesize; i += 2) {
        *to++ = *(source.data + i);
    }

    const size_t width = source.width * 2;  // in byte
    const size_t height = source.height;

    auto copy_u_or_v = [&width, &height, &to](TFV_ImageData const* u_or_v_ptr) {

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

int constexpr tfv::YUVToRGB::coeff_r[];
int constexpr tfv::YUVToRGB::coeff_g[];
int constexpr tfv::YUVToRGB::coeff_b[];

void tfv::YUVToRGB::target_size(tfv::Image const& source, size_t& target_width,
                                size_t& target_height,
                                size_t& target_bytesize) const {
    target_width = source.width;
    target_height = source.height;
    target_bytesize = (source.width * source.height) * 3;  // 24 bit/pixel
}

template <size_t r, size_t g, size_t b>
void tfv::YUVToRGB::convert(int const y, int const u, int const v,
                            TFV_ImageData* rgb) const {

    // need indices 0,1,2
    static_assert(((r + g + b) == 3) and (r < 3) and (g < 3) and (b < 3) and
                      ((r == 0) or (g == 0) or (b == 0)),
                  "Need to provide rgb channels as 0, 1 and 2");

    *(rgb + r) =
        clamp_((coeff_r[0] * y + coeff_r[1] * u + coeff_r[2] * v) / normalizer);

    *(rgb + g) =
        clamp_((coeff_g[0] * y + coeff_g[1] * u + coeff_g[2] * v) / normalizer);

    *(rgb + b) =
        clamp_((coeff_b[0] * y + coeff_b[1] * u + coeff_b[2] * v) / normalizer);
}

template <size_t r, size_t g, size_t b>
void tfv::YUYVToRGBType::convert(tfv::Image const& source,
                                 tfv::Image& target) const {

    auto to = target.data;

    for (auto src = source.data; src < (source.data + source.bytesize);) {

        int const y1 = static_cast<int>(*src++) - 16;
        int const u = static_cast<int>(*src++) - 128;
        int const y2 = static_cast<int>(*src++) - 16;
        int const v = static_cast<int>(*src++) - 128;

        YUVToRGB::convert<r, g, b>(y1, u, v, to);
        to += 3;
        YUVToRGB::convert<r, g, b>(y2, u, v, to);
        to += 3;
    }
}

tfv::ImageFormat tfv::ConvertYUYVToRGB::target_format(
    tfv::Image const& source, size_t& target_width, size_t& target_height,
    size_t& target_bytesize) const {
    target_size(source, target_width, target_height, target_bytesize);
    return tfv::ImageFormat::RGB888;
}

tfv::ImageFormat tfv::ConvertYUYVToBGR::target_format(
    tfv::Image const& source, size_t& target_width, size_t& target_height,
    size_t& target_bytesize) const {
    target_size(source, target_width, target_height, target_bytesize);
    return tfv::ImageFormat::BGR888;
}

template <size_t r, size_t g, size_t b>
void tfv::YV12ToRGBType::convert(tfv::Image const& source,
                                 tfv::Image& target) const {

    auto to = target.data;
    auto const v_plane = source.data + source.width * source.height;
    auto const u_plane = v_plane + ((source.width * source.height) >> 2);
    auto const uv_offset = source.width >> 1;

    // abbreviated version of http://en.wikipedia.org/wiki/YUV
    for (size_t i = 0; i < source.height; i++) {
        auto const row_uv = (i >> 1) * uv_offset;
        auto const row_y = i * source.width;
        for (size_t j = 0; j < source.width; j++) {

            // Every u(v)-value corresponds to one four-block of
            // Y-values,
            // i.e. each two y's from subsequent rows. According to
            // this,
            // the index of the u(v)-value needed is given by:
            // uv_idx = ((i / 2) * (source.width / 2)) + (j / 2)
            // (i.e. like indexing a 2d array with one idx).
            // With the definition of row_uv it can be written:

            auto uv_idx = (j >> 1) + row_uv;

            // std::cout << row_y + j << "," << uv_idx << std::endl;

            int const y = static_cast<int>(source.data[row_y + j]) - 16;
            int const u = static_cast<int>(u_plane[uv_idx]) - 128;
            int const v = static_cast<int>(v_plane[uv_idx]) - 128;

            YUVToRGB::convert<r, g, b>(y, u, v, to);
            to += 3;
        }
    }
}

tfv::ImageFormat tfv::ConvertYV12ToRGB::target_format(
    tfv::Image const& source, size_t& target_width, size_t& target_height,
    size_t& target_bytesize) const {
    target_size(source, target_width, target_height, target_bytesize);
    return tfv::ImageFormat::RGB888;
}

void tfv::ConvertYV12ToRGB::convert(tfv::Image const& source,
                                    tfv::Image& target) const {
    tfv::YV12ToRGBType::convert<0, 1, 2>(source, target);
}

tfv::ImageFormat tfv::ConvertYV12ToBGR::target_format(
    tfv::Image const& source, size_t& target_width, size_t& target_height,
    size_t& target_bytesize) const {
    target_size(source, target_width, target_height, target_bytesize);
    return tfv::ImageFormat::BGR888;
}

void tfv::ConvertYV12ToBGR::convert(tfv::Image const& source,
                                    tfv::Image& target) const {
    tfv::YV12ToRGBType::convert<2, 1, 0>(source, target);
}

void tfv::RGBTypeConversion::target_size(tfv::Image const& source,
                                         size_t& target_width,
                                         size_t& target_height,
                                         size_t& target_bytesize) const {
    target_width = source.width;
    target_height = source.height;
    target_bytesize = source.bytesize;
}

void tfv::RGBFromToBGR::target_size(tfv::Image const& source,
                                    size_t& target_width, size_t& target_height,
                                    size_t& target_bytesize) const {
    target_width = source.width;
    target_height = source.height;
    target_bytesize = source.bytesize;
}

void tfv::RGBFromToBGR::convert(tfv::Image const& source,
                                tfv::Image& target) const {
    auto to = target.data;
    for (size_t i = 0; i < source.bytesize; i += 3) {
        *to++ = source.data[i + 2];
        *to++ = source.data[i + 1];
        *to++ = source.data[i];
    }
}

tfv::ImageFormat tfv::ConvertRGBToBGR::target_format(
    tfv::Image const& source, size_t& target_width, size_t& target_height,
    size_t& target_bytesize) const {

    target_size(source, target_width, target_height, target_bytesize);
    return tfv::ImageFormat::BGR888;
}

tfv::ImageFormat tfv::ConvertBGRToRGB::target_format(
    tfv::Image const& source, size_t& target_width, size_t& target_height,
    size_t& target_bytesize) const {

    target_size(source, target_width, target_height, target_bytesize);
    return tfv::ImageFormat::RGB888;
}

tfv::Converter::Converter(tfv::ImageFormat source, tfv::ImageFormat target) {
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

tfv::Converter::~Converter(void) {
    if (converter_) {
        delete converter_;
    }
}

tfv::Image const& tfv::Converter::operator()(tfv::Image const& source) const {
    if (not converter_) {
        return invalid_image_;
    }

    return (*converter_)(source);
}
