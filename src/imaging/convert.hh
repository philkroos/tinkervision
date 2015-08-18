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
#include <limits>

#include "image.hh"
#include "tinkervision_defines.h"

namespace tfv {

/** Restrict a value of type In to the range of type Out.
 */
template <typename In, typename Out>
struct Clamp {
private:
    In const min_{static_cast<In>(std::numeric_limits<Out>::min())};
    In const max_{static_cast<In>(std::numeric_limits<Out>::max())};

public:
    Out constexpr operator()(In const& value) {
        return static_cast<Out>((value < min_) ? min_ : (value > max_) ? max_
                                                                       : value);
    }
};

/**
 * Restrict a value to the range of TFV_ImageData.
 */
template <typename In>
struct ClampImageValue : public Clamp<In, TFV_ImageData> {};

// forward declaration of Convert-wrapper (providing public interface to this
// module)
class Converter;

/**
 * Baseclass of all converters
 */
struct Convert {
    Convert(ColorSpace source, ColorSpace target);
    virtual ~Convert(void) {
        if (target) {
            delete target;
        }
    }

    Convert(Convert const&) = delete;
    Convert(Convert const&&) = delete;
    Convert& operator=(Convert const&) = delete;
    Convert& operator=(Convert const&&) = delete;

    Image const& operator()(Image const& source);
    void operator()(Image const& source, Image& target);

protected:
    virtual void target_format(Image const& source, size_t& target_width,
                               size_t& target_height,
                               size_t& target_bytesize) const = 0;
    virtual void convert(Image const& source, Image& target) const = 0;

private:
    friend class Converter;
    Image* target{nullptr};
    ColorSpace const source_format_;
    ColorSpace const target_format_;
};

//
// Following: Converter from YCbCr to ...
//

/**
 * This converter is compressing the data.
 */
struct ConvertYUV422ToYUV420 : public Convert {
public:
    ConvertYUV422ToYUV420(void);
    virtual ~ConvertYUV422ToYUV420(void) = default;

protected:
    virtual void target_format(Image const& source, size_t& target_width,
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
public:
    virtual ~ConvertYUYVToYV12(void) = default;

protected:
    virtual ColorSpace source_format(void) const { return ColorSpace::YUYV; }
    virtual void convert(Image const& source, Image& target) const {
        convert_yuyv(source, target);
    }
};

/**
 * According to [Kaufmann], the conversion goes like this:
 * |R|    1  |298.082  0       458.942|   |Y' - 16 |
 * |G| =  -  |298.082 -54.592 -136.425| * |Cb - 128|
 * |B|   256 |298.082  540.775 0      |   |Cr - 128|
 * (p319) for HDTV (>= 1280x720)
 * and Wikipedia says (http://en.wikipedia.org/wiki/YUV):
 * r = y + 1.28033 * v
 * g = y - 0.21482 * u - 0.38059 * v
 * b = y + 2.21798 * u
 * for HD and for SD:
 * r = y + 1.3983 * v
 * g = y - 0.39465 * u - 0.58060 * v
 * b = y + 2.03211 * u
 * All are citing the same standard (BT.601 for SD, BT.709 for HD).
 * Confusing. All seem to work fine.
 * [Kaufmann] - Digital Video and HDTV Algorithms ... p313ff
 */
struct YUVToRGB {
public:
    virtual ~YUVToRGB(void) {}

private:
    ClampImageValue<double> clamp_;

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
public:
    virtual ~YUYVToRGBType(void) = default;

protected:
    template <size_t r, size_t g, size_t b>
    void convert(Image const& source, Image& target) const;
};

struct ConvertYUYVToRGB : public Convert, public YUYVToRGBType {
public:
    ConvertYUYVToRGB(void);
    virtual ~ConvertYUYVToRGB(void) = default;

protected:
    virtual ColorSpace source_format(void) const { return ColorSpace::YUYV; }
    virtual void target_format(Image const& source, size_t& target_width,
                               size_t& target_height,
                               size_t& target_bytesize) const;

    virtual void convert(Image const& source, Image& target) const {
        YUYVToRGBType::convert<0, 1, 2>(source, target);
    }
};

struct ConvertYUYVToBGR : public Convert, public YUYVToRGBType {
public:
    ConvertYUYVToBGR(void);
    virtual ~ConvertYUYVToBGR(void) = default;

protected:
    virtual ColorSpace source_format(void) const { return ColorSpace::YUYV; }
    virtual void target_format(Image const& source, size_t& target_width,
                               size_t& target_height,
                               size_t& target_bytesize) const;

    virtual void convert(Image const& source, Image& target) const {
        YUYVToRGBType::convert<2, 1, 0>(source, target);
    }
};

struct YV12ToRGBType : public Convert, public YUVToRGB {
public:
    YV12ToRGBType(ColorSpace to);
    virtual ~YV12ToRGBType(void) = default;

protected:
    virtual ColorSpace source_format(void) const { return ColorSpace::YV12; }

    template <size_t r, size_t g, size_t b>
    void convert(Image const& source, Image& target) const;
};

struct ConvertYV12ToRGB : public YV12ToRGBType {
public:
    ConvertYV12ToRGB(void);
    virtual ~ConvertYV12ToRGB(void) = default;

protected:
    virtual void target_format(Image const& source, size_t& target_width,
                               size_t& target_height,
                               size_t& target_bytesize) const;

    virtual void convert(Image const& source, Image& target) const;
};

struct ConvertYV12ToBGR : public YV12ToRGBType {
public:
    ConvertYV12ToBGR(void);
    virtual ~ConvertYV12ToBGR(void) = default;

protected:
    virtual void target_format(Image const& source, size_t& target_width,
                               size_t& target_height,
                               size_t& target_bytesize) const;

    virtual void inline convert(Image const& source, Image& target) const;
};

//
// Following: Converter from RGB to ...
//

struct RGBFromToBGR : public Convert {
public:
    RGBFromToBGR(ColorSpace from, ColorSpace to);
    virtual ~RGBFromToBGR(void) = default;

protected:
    void target_size(Image const& source, size_t& target_width,
                     size_t& target_height, size_t& target_bytesize) const;

    virtual void convert(Image const& source, Image& target) const;
};

struct ConvertRGBToBGR : public RGBFromToBGR {
public:
    ConvertRGBToBGR(void);
    virtual ~ConvertRGBToBGR(void) = default;

protected:
    virtual ColorSpace source_format(void) const { return ColorSpace::RGB888; }

    virtual void target_format(Image const& source, size_t& target_width,
                               size_t& target_height,
                               size_t& target_bytesize) const;
};

//
// Following: Converter from BGR to ...
//

struct ConvertBGRToRGB : public RGBFromToBGR {
public:
    ConvertBGRToRGB(void);
    virtual ~ConvertBGRToRGB(void) = default;

protected:
    virtual ColorSpace source_format(void) const { return ColorSpace::BGR888; }

    virtual void target_format(Image const& source, size_t& target_width,
                               size_t& target_height,
                               size_t& target_bytesize) const;
};

struct ConvertBGRToYV12 : public Convert {
public:
    ConvertBGRToYV12(void);
    virtual ~ConvertBGRToYV12(void) = default;

protected:
    virtual ColorSpace source_format(void) const { return ColorSpace::BGR888; }

    virtual void target_format(Image const& source, size_t& target_width,
                               size_t& target_height,
                               size_t& target_bytesize) const final;

    virtual void convert(Image const& source, Image& target) const final;
};

struct ConvertBGRToYUYV : public Convert {
public:
    ConvertBGRToYUYV(void);
    virtual ~ConvertBGRToYUYV(void) = default;

protected:
    virtual ColorSpace source_format(void) const { return ColorSpace::BGR888; }

    virtual void target_format(Image const& source, size_t& target_width,
                               size_t& target_height,
                               size_t& target_bytesize) const final;

    virtual void convert(Image const& source, Image& target) const final;
};

//
// Following: Converter from Gray to ...
//

struct ConvertGrayToBGR888 : public Convert {
public:
    ConvertGrayToBGR888(void);
    virtual ~ConvertGrayToBGR888(void) = default;

protected:
    void target_format(Image const& source, size_t& target_width,
                       size_t& target_height,
                       size_t& target_bytesize) const override final;

    void convert(Image const& source, Image& target) const override final;
};

/**
 * Public interface to this module
 */
class Converter {
private:
    Convert* converter_;
    Image const invalid_image_{};

    using Conversion =
        std::tuple<ColorSpace, ColorSpace, Convert* (Converter::*)(void)>;

    using Conversions = std::vector<Conversion>;

    Conversions const conversions_{
        std::make_tuple(ColorSpace::YUYV, ColorSpace::YV12,
                        &Converter::yuyv_to_yv12),
        std::make_tuple(ColorSpace::YUYV, ColorSpace::BGR888,
                        &Converter::yuyv_to_bgr),
        std::make_tuple(ColorSpace::YUYV, ColorSpace::RGB888,
                        &Converter::yuyv_to_rgb),
        std::make_tuple(ColorSpace::YV12, ColorSpace::RGB888,
                        &Converter::yv12_to_rgb),
        std::make_tuple(ColorSpace::YV12, ColorSpace::BGR888,
                        &Converter::yv12_to_bgr),
        std::make_tuple(ColorSpace::BGR888, ColorSpace::RGB888,
                        &Converter::bgr_to_rgb),
        std::make_tuple(ColorSpace::RGB888, ColorSpace::BGR888,
                        &Converter::rgb_to_bgr),
        std::make_tuple(ColorSpace::BGR888, ColorSpace::YV12,
                        &Converter::bgr_to_yv12),
        std::make_tuple(ColorSpace::BGR888, ColorSpace::YUYV,
                        &Converter::bgr_to_yuyv),
        std::make_tuple(ColorSpace::GRAY, ColorSpace::BGR888,
                        &Converter::gray_to_bgr)};

    Convert* yuyv_to_yv12(void) { return new ConvertYUYVToYV12(); }
    Convert* yuyv_to_bgr(void) { return new ConvertYUYVToBGR(); }
    Convert* yuyv_to_rgb(void) { return new ConvertYUYVToRGB(); }
    Convert* yv12_to_rgb(void) { return new ConvertYV12ToRGB(); }
    Convert* yv12_to_bgr(void) { return new ConvertYV12ToBGR(); }
    Convert* bgr_to_rgb(void) { return new ConvertBGRToRGB(); }
    Convert* rgb_to_bgr(void) { return new ConvertRGBToBGR(); }
    Convert* bgr_to_yv12(void) { return new ConvertBGRToYV12(); }
    Convert* bgr_to_yuyv(void) { return new ConvertBGRToYUYV(); }
    Convert* gray_to_bgr(void) { return new ConvertBGRToYUYV(); }

private:  // These should really be just deleted, but current compiler has a
          // bug. See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=58249

public:
    Converter(ColorSpace source, ColorSpace target);

    /**
     * Providing move constructor to let this class be stored in STL-containers.
     */
    Converter(Converter&& other) {
        this->converter_ = other.converter_;
        other.converter_ = nullptr;
    }

    /**
     * Providing copy constructor to let this class be stored in STL-containers.
     */
    Converter(Converter& other) {
        this->converter_ = other.converter_;
        other.converter_ = nullptr;
    }
    Converter& operator=(Converter const&) = delete;
    Converter& operator=(Converter const&&) = delete;

    ~Converter(void);

    // static bool known_conversion(ColorSpace source, ColorSpace target);

    Image const& operator()(Image const& source) const;
    void operator()(Image const& source, Image& target) const;

    Image const& result(void) const {
        return (converter_ and converter_->target) ? *(converter_->target)
                                                   : invalid_image_;
    }

    ColorSpace target_format(void) const {
        return converter_ ? converter_->target_format_
                          : tfv::ColorSpace::INVALID;
    }

    ColorSpace source_format(void) const {
        return converter_ ? converter_->source_format_
                          : tfv::ColorSpace::INVALID;
    }
};
}
#endif /* CONVERT_H */
