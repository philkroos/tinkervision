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

#ifndef CONVERT_H
#define CONVERT_H

#include <array>
#include <vector>
#include <tuple>

#include "image.hh"
#include "tinkervision_defines.h"

namespace tfv {

template <typename In, typename Out, unsigned min, unsigned max>
struct Clamp {
    Out constexpr operator()(In const& value) {
        return static_cast<Out>((value < min) ? min : (value > max) ? max
                                                                    : value);
    }
};

class Converter;

struct Convert {
    Convert(void){};
    virtual ~Convert(void);
    Convert(Convert const&) = delete;
    Convert(Convert const&&) = delete;
    Convert& operator=(Convert const&) = delete;
    Convert& operator=(Convert const&&) = delete;

    Image const& operator()(Image const& source);

protected:
    virtual ImageFormat source_format(void) const = 0;
    virtual ImageFormat target_format(Image const& source, size_t& target_width,
                                      size_t& target_height,
                                      size_t& target_bytesize) const = 0;
    virtual void convert(Image const& source, Image& target) const = 0;

private:
    friend class Converter;
    Image* target = nullptr;
    ImageFormat target_format_ = ImageFormat::INVALID;
};

/**
 * This converter is compressing the data.
 */
struct ConvertYUV422ToYUV420 : public Convert {
protected:
    virtual ImageFormat target_format(Image const& source, size_t& target_width,
                                      size_t& target_height,
                                      size_t& target_bytesize) const;

    virtual void convert_yuyv(Image const& source, Image& target) const {
        convert_any(source, target, source.data + 1, source.data + 3);
    }

    virtual void convert_yvyu(Image const& source, Image& target) const {
        convert_any(source, target, source.data + 3, source.data + 1);
    }

    // output is in order y-block, v-block, u-block
    virtual void convert_any(Image const& source, Image& target,
                             TFV_ImageData* u_ptr, TFV_ImageData* v_ptr) const;
};

struct ConvertYUYVToYV12 : public ConvertYUV422ToYUV420 {
protected:
    virtual ImageFormat source_format(void) const { return ImageFormat::YUYV; }
    virtual void convert(Image const& source, Image& target) const {
        convert_yuyv(source, target);
    }
};

/**
 * Y'CbCr in colorspace sRGB according to
 * http://linuxtv.org/downloads/v4l-dvb-apis/ch02s06.html#col-srgb:
 * Y' = 0.2990R' + 0.5870G' + 0.1140B'
 * Cb = -0.1687R' - 0.3313G' + 0.5B'
 * Cr = 0.5R' - 0.4187G' - 0.0813B'
 * but this is studio RGB according to [Kaufmann]. He puts:
 * |R|    1  |298.082  0       458.942|   |Y' - 16 |
 * |G| =  -  |298.082 -54.592 -136.425| * |Cb - 128|
 * |B|   256 |298.082  540.775 0      |   |Cr - 128|
 */
struct YUVToRGB {
private:
    Clamp<double, TFV_ImageData, 0, 255> clamp_;
    int constexpr static coeff_r[3] = {298082, 0, 458942};
    int constexpr static coeff_g[3] = {298082, -54592, -136425};
    int constexpr static coeff_b[3] = {298082, 540775, 0};
    double constexpr static normalizer = 256000.0;

protected:
    void target_size(Image const& source, size_t& target_width,
                     size_t& target_height, size_t& target_bytesize) const;

public:
    template <size_t r = 0, size_t g = 1, size_t b = 2>
    void convert(int const y, int const u, int const v,
                 TFV_ImageData* rgb) const;
};

struct YUYVToRGBType : public YUVToRGB {

protected:
    template <size_t r, size_t g, size_t b>
    void convert(Image const& source, Image& target) const;
};

struct ConvertYUYVToRGB : public Convert, public YUYVToRGBType {

protected:
    virtual ImageFormat source_format(void) const { return ImageFormat::YUYV; }
    virtual ImageFormat target_format(Image const& source, size_t& target_width,
                                      size_t& target_height,
                                      size_t& target_bytesize) const;

    virtual void convert(Image const& source, Image& target) const {
        YUYVToRGBType::convert<0, 1, 2>(source, target);
    }
};

struct ConvertYUYVToBGR : public Convert, public YUYVToRGBType {

protected:
    virtual ImageFormat source_format(void) const { return ImageFormat::YUYV; }
    virtual ImageFormat target_format(Image const& source, size_t& target_width,
                                      size_t& target_height,
                                      size_t& target_bytesize) const;

    virtual void convert(Image const& source, Image& target) const {
        YUYVToRGBType::convert<2, 1, 0>(source, target);
    }
};

struct YV12ToRGBType : public Convert, public YUVToRGB {
protected:
    virtual ImageFormat source_format(void) const { return ImageFormat::YV12; }

    template <size_t r, size_t g, size_t b>
    void convert(Image const& source, Image& target) const;
};

struct ConvertYV12ToRGB : public YV12ToRGBType {

protected:
    virtual ImageFormat target_format(Image const& source, size_t& target_width,
                                      size_t& target_height,
                                      size_t& target_bytesize) const;

    virtual void convert(Image const& source, Image& target) const;
};

struct ConvertYV12ToBGR : public YV12ToRGBType {

protected:
    virtual ImageFormat target_format(Image const& source, size_t& target_width,
                                      size_t& target_height,
                                      size_t& target_bytesize) const;

    virtual void inline convert(Image const& source, Image& target) const;
};

struct RGBTypeConversion : public Convert {
protected:
    void target_size(Image const& source, size_t& target_width,
                     size_t& target_height, size_t& target_bytesize) const;
};

struct RGBFromToBGR : public Convert {
protected:
    void target_size(Image const& source, size_t& target_width,
                     size_t& target_height, size_t& target_bytesize) const;

    virtual void convert(Image const& source, Image& target) const;
};

struct ConvertRGBToBGR : public RGBFromToBGR {
public:
    ConvertRGBToBGR(void);
    virtual ~ConvertRGBToBGR(void);

protected:
    virtual ImageFormat source_format(void) const {
        return ImageFormat::RGB888;
    }

    virtual ImageFormat target_format(Image const& source, size_t& target_width,
                                      size_t& target_height,
                                      size_t& target_bytesize) const;
};

struct ConvertBGRToRGB : public RGBFromToBGR {
protected:
    virtual ImageFormat source_format(void) const {
        return ImageFormat::BGR888;
    }

    virtual ImageFormat target_format(Image const& source, size_t& target_width,
                                      size_t& target_height,
                                      size_t& target_bytesize) const;
};

class Converter {
private:
    Convert* converter_;
    Image const invalid_image_{};

    using Conversion =
        std::tuple<ImageFormat, ImageFormat, Convert* (Converter::*)(void)>;

    using Conversions = std::vector<Conversion>;

    Conversions const conversions_{
        std::make_tuple(ImageFormat::YUYV, ImageFormat::YV12,
                        &Converter::yuyv_to_yv12),
        std::make_tuple(ImageFormat::YUYV, ImageFormat::BGR888,
                        &Converter::yuyv_to_bgr),
        std::make_tuple(ImageFormat::YUYV, ImageFormat::RGB888,
                        &Converter::yuyv_to_rgb),
        std::make_tuple(ImageFormat::YV12, ImageFormat::RGB888,
                        &Converter::yv12_to_rgb),
        std::make_tuple(ImageFormat::YV12, ImageFormat::BGR888,
                        &Converter::yv12_to_bgr),
        std::make_tuple(ImageFormat::BGR888, ImageFormat::RGB888,
                        &Converter::bgr_to_rgb),
        std::make_tuple(ImageFormat::RGB888, ImageFormat::BGR888,
                        &Converter::rgb_to_bgr)};

    Convert* yuyv_to_yv12(void) { return new ConvertYUYVToYV12(); }
    Convert* yuyv_to_bgr(void) { return new ConvertYUYVToBGR(); }
    Convert* yuyv_to_rgb(void) { return new ConvertYUYVToRGB(); }
    Convert* yv12_to_rgb(void) { return new ConvertYV12ToRGB(); }
    Convert* yv12_to_bgr(void) { return new ConvertYV12ToBGR(); }
    Convert* bgr_to_rgb(void) { return new ConvertBGRToRGB(); }
    Convert* rgb_to_bgr(void) { return new ConvertRGBToBGR(); }

private:  // These should really be just deleted, but current compiler has a
          // bug. See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=58249

public:
    Converter(ImageFormat source, ImageFormat target);
    Converter(Converter&& other) {
        this->converter_ = other.converter_;
        other.converter_ = nullptr;
    }
    Converter(Converter& other) {
        this->converter_ = other.converter_;
        other.converter_ = nullptr;
    }
    Converter& operator=(Converter const&) = delete;
    Converter& operator=(Converter const&&) = delete;

    // Converter(Converter const&) = delete;
    // Converter(Converter const&&) = delete;
    // Converter& operator=(Converter const&) = delete;
    // Converter& operator=(Converter const&&) = delete;

    ~Converter(void);

    Image const& operator()(Image const& source) const;

    Image const& result(void) const {
        return (converter_ and converter_->target) ? *(converter_->target)
                                                   : invalid_image_;
    }

    ImageFormat target_format(void) const {
        return converter_ ? converter_->target_format_
                          : tfv::ImageFormat::INVALID;
    }
};
}
#endif /* CONVERT_H */
