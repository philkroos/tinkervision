/// \file convert.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Declaration of the different colorspace converters of Tinkervision.
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

#ifndef CONVERT_H
#define CONVERT_H

#include <cmath>
#include <array>
#include <vector>
#include <algorithm>
#include <tuple>
#include <limits>
#include <cassert>

#include "image.hh"
#include "tinkervision_defines.h"
#include "logger.hh"

namespace tv {

/// Restrict a value of type In to the range of type Out.
template <typename In, typename Out>
struct Clamp {
private:
    In const min_{static_cast<In>(std::numeric_limits<Out>::min())};
    In const max_{static_cast<In>(std::numeric_limits<Out>::max())};

public:
    Out constexpr operator()(In const& value) const {
        return static_cast<Out>(
            std::round((value < min_) ? min_ : (value > max_) ? max_ : value));
    }
};

/// Restrict a value to the range of uint8_t.
template <typename In>
struct ClampImageValue : public Clamp<In, uint8_t> {};

// forward declaration of Convert-wrapper (providing public interface to this
// module)
class Converter;

/// Baseclass of all converters
struct Convert {
    Convert(ColorSpace source, ColorSpace target);
    virtual ~Convert(void);

    Convert(Convert const&) = delete;
    Convert(Convert const&&) = delete;
    Convert& operator=(Convert const&) = delete;
    Convert& operator=(Convert const&&) = delete;

    Image const& operator()(Image const& source);
    void operator()(Image const& source, Image& target);

    ImageHeader convert_header(ImageHeader const& source);

protected:
    virtual void target_format(ImageHeader const& source,
                               uint16_t& target_width, uint16_t& target_height,
                               size_t& target_bytesize) const = 0;
    virtual void convert(Image const& source, Image& target) const = 0;

private:
    friend class Converter;
    Image target;

    ColorSpace const source_format_;
    ColorSpace const target_format_;
};

//
// Following: Converter from YCbCr to ...
//

/// This converter is compressing the data.
struct ConvertYUV422ToYUV420 : public Convert {
public:
    ConvertYUV422ToYUV420(void);
    ~ConvertYUV422ToYUV420(void) override = default;

protected:
    void target_format(ImageHeader const& source, uint16_t& target_width,
                       uint16_t& target_height,
                       size_t& target_bytesize) const override final;

    void convert_yuyv(Image const& source, Image& target) const {
        convert_any(source, target, source.data + 1, source.data + 3);
    }

    void convert_yvyu(Image const& source, Image& target) const {
        convert_any(source, target, source.data + 3, source.data + 1);
    }

    // output is in order y-block, v-block, u-block
    void convert_any(Image const& source, Image& target, uint8_t* u_ptr,
                     uint8_t* v_ptr) const;
};

struct ConvertYUYVToYV12 : public ConvertYUV422ToYUV420 {
public:
    ~ConvertYUYVToYV12(void) override final = default;

protected:
    void convert(Image const& source, Image& target) const override final {
        assert(source.header.format == ColorSpace::YUYV);
        convert_yuyv(source, target);
    }
};

/// According to [Kaufmann], the conversion goes like this:
/// |R|    1  |298.082  0       458.942|   |Y' - 16 |
/// |G| =  -  |298.082 -54.592 -136.425| * |Cb - 128|
/// |B|   256 |298.082  540.775 0      |   |Cr - 128|
/// (p319) for HDTV (>= 1280x720)
/// and Wikipedia says (http://en.wikipedia.org/wiki/YUV):
/// r = y + 1.28033 * v
/// g = y - 0.21482 * u - 0.38059 * v
/// b = y + 2.21798 * u
/// for HD and for SD:
/// r = y + 1.3983 * v
/// g = y - 0.39465 * u - 0.58060 * v * b = y + 2.03211 * u
/// All are citing the same standard (BT.601 for SD, BT.709 for HD).
/// Confusing. All seem to work fine.
/// [Kaufmann] - Digital Video and HDTV Algorithms ... p313ff
struct YUVToRGB {
public:
    virtual ~YUVToRGB(void) = default;

private:
    ClampImageValue<double> clamp_;

    int constexpr static coeff_r[3] = {298082, 0, 458942};
    int constexpr static coeff_g[3] = {298082, -54592, -136425};
    int constexpr static coeff_b[3] = {298082, 540775, 0};
    double constexpr static normalizer = 256000.0;

protected:
    void target_size(ImageHeader const& source, uint16_t& target_width,
                     uint16_t& target_height, size_t& target_bytesize) const;

public:
    template <size_t r = 0, size_t g = 1, size_t b = 2>
    void convert(int const y, int const u, int const v, uint8_t* rgb) const;
};

struct YUYVToRGBType : public YUVToRGB {
public:
    ~YUYVToRGBType(void) override = default;

protected:
    template <size_t r, size_t g, size_t b>
    void convert(Image const& source, Image& target) const;
};

struct ConvertYUYVToRGB : public Convert, public YUYVToRGBType {
public:
    ConvertYUYVToRGB(void);
    ~ConvertYUYVToRGB(void) override final = default;

protected:
    void target_format(ImageHeader const& source, uint16_t& target_width,
                       uint16_t& target_height,
                       size_t& target_bytesize) const override final;

    void convert(Image const& source, Image& target) const override final {
        assert(source.header.format == ColorSpace::YUYV);
        YUYVToRGBType::convert<0, 1, 2>(source, target);
    }
};

struct ConvertYUYVToBGR : public Convert, public YUYVToRGBType {
public:
    ConvertYUYVToBGR(void);
    ~ConvertYUYVToBGR(void) override final = default;

protected:
    void target_format(ImageHeader const& source, uint16_t& target_width,
                       uint16_t& target_height,
                       size_t& target_bytesize) const override final;

    void convert(Image const& source, Image& target) const override final {
        assert(source.header.format == ColorSpace::YUYV);
        YUYVToRGBType::convert<2, 1, 0>(source, target);
    }
};

struct YV12ToRGBType : public Convert, public YUVToRGB {
public:
    YV12ToRGBType(ColorSpace to);
    ~YV12ToRGBType(void) override = default;

protected:
    template <uint8_t r, uint8_t g, uint8_t b>
    void convert(Image const& source, Image& target) const;
};

/// Convert from Y'V420p to BGR888.
/// Uses formula from [wiki], section Y'UV420p (and Y'V12 or YV12) to RGB888
/// conversion
/// [wiki]: https://en.wikipedia.org/wiki/YUV
struct ConvertYV12ToRGB : public YV12ToRGBType {
public:
    ConvertYV12ToRGB(void);
    ~ConvertYV12ToRGB(void) override final = default;

protected:
    void target_format(ImageHeader const& source, uint16_t& target_width,
                       uint16_t& target_height,
                       size_t& target_bytesize) const override final;

    void convert(Image const& source, Image& target) const override final;
};

struct ConvertYV12ToBGR : public YV12ToRGBType {
public:
    ConvertYV12ToBGR(void);
    ~ConvertYV12ToBGR(void) override final = default;

protected:
    void target_format(ImageHeader const& source, uint16_t& target_width,
                       uint16_t& target_height,
                       size_t& target_bytesize) const override final;

    void inline convert(Image const& source,
                        Image& target) const override final;
};

//
// Following: Converter from RGB to ...
//

struct RGBFromToBGR : public Convert {
public:
    RGBFromToBGR(ColorSpace from, ColorSpace to);
    ~RGBFromToBGR(void) override = default;

protected:
    void target_size(ImageHeader const& source, uint16_t& target_width,
                     uint16_t& target_height, size_t& target_bytesize) const;

    void convert(Image const& source, Image& target) const override final;
};

struct ConvertRGBToBGR : public RGBFromToBGR {
public:
    ConvertRGBToBGR(void);
    ~ConvertRGBToBGR(void) override final = default;

protected:
    void target_format(ImageHeader const& source, uint16_t& target_width,
                       uint16_t& target_height,
                       size_t& target_bytesize) const override final;
};

//
// Following: Converter from BGR to ...
//

struct ConvertBGRToRGB : public RGBFromToBGR {
public:
    ConvertBGRToRGB(void);
    ~ConvertBGRToRGB(void) override final = default;

protected:
    virtual void target_format(ImageHeader const& source,
                               uint16_t& target_width, uint16_t& target_height,
                               size_t& target_bytesize) const;
};

/// Convert from BGR888 to Y'V420p.
/// Uses formula and description from [wiki], sections
/// "Y'UV420p (and Y'V12 or YV12) to RGB888 conversion" and "Y'UV444 to RGB888
/// conversion".
/// [wiki]: https://en.wikipedia.org/wiki/YUV
struct ConvertBGRToYV12 : public Convert {
public:
    ConvertBGRToYV12(void);
    ~ConvertBGRToYV12(void) override final = default;

protected:
    void target_format(ImageHeader const& source, uint16_t& target_width,
                       uint16_t& target_height,
                       size_t& target_bytesize) const override final;

    void convert(Image const& source, Image& target) const override final;
};

/// Convert from BGR888 to Y'UV444.
/// Uses formula and description from [wiki], section
/// "Y'UV444 to RGB888 conversion".
/// [wiki]: https://en.wikipedia.org/wiki/YUV
struct ConvertBGRToYUYV : public Convert {
public:
    ConvertBGRToYUYV(void);
    ~ConvertBGRToYUYV(void) override final = default;

protected:
    void target_format(ImageHeader const& source, uint16_t& target_width,
                       uint16_t& target_height,
                       size_t& target_bytesize) const override final;

    void convert(Image const& source, Image& target) const override final;
};

struct ConvertBGRToGray : public Convert {
private:
    ClampImageValue<double> clamp_;

public:
    ConvertBGRToGray(void);
    ~ConvertBGRToGray(void) override = default;

protected:
    void target_format(ImageHeader const& source, uint16_t& target_width,
                       uint16_t& target_height,
                       size_t& target_bytesize) const override final;

    /// This uses the conversion routine as described by OpenCV:
    /// http://docs.opencv.org/modules/imgproc/doc/miscellaneous_transformations.html#cvtcolor
    void convert(Image const& source, Image& target) const override final;
};

//
// Following: Converter from Gray to ...
// This converters do not introduce any color, they just convert the image
// back to three channels.
//

struct ConvertGrayToBGR : public Convert {
public:
    ConvertGrayToBGR(void);
    ~ConvertGrayToBGR(void) override = default;

protected:
    void target_format(ImageHeader const& source, uint16_t& target_width,
                       uint16_t& target_height,
                       size_t& target_bytesize) const override final;

    void convert(Image const& source, Image& target) const override final;
};

/// Public interface to this module.
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
                        &Converter::gray_to_bgr),
        std::make_tuple(ColorSpace::BGR888, ColorSpace::GRAY,
                        &Converter::bgr_to_gray)};

    Convert* yuyv_to_yv12(void) { return new ConvertYUYVToYV12(); }
    Convert* yuyv_to_bgr(void) { return new ConvertYUYVToBGR(); }
    Convert* yuyv_to_rgb(void) { return new ConvertYUYVToRGB(); }
    Convert* yv12_to_rgb(void) { return new ConvertYV12ToRGB(); }
    Convert* yv12_to_bgr(void) { return new ConvertYV12ToBGR(); }
    Convert* bgr_to_rgb(void) { return new ConvertBGRToRGB(); }
    Convert* rgb_to_bgr(void) { return new ConvertRGBToBGR(); }
    Convert* bgr_to_yv12(void) { return new ConvertBGRToYV12(); }
    Convert* bgr_to_yuyv(void) { return new ConvertBGRToYUYV(); }
    Convert* gray_to_bgr(void) { return new ConvertGrayToBGR(); }
    Convert* bgr_to_gray(void) { return new ConvertBGRToGray(); }

public:
    Converter(ColorSpace source, ColorSpace target);

    /// Providing move constructor to let this class be stored in
    /// STL-containers.
    Converter(Converter&& other) {
        this->converter_ = other.converter_;
        other.converter_ = nullptr;
    }

    /// Providing copy constructor to let this class be stored in
    /// STL-containers.
    Converter(Converter& other) {
        this->converter_ = other.converter_;
        other.converter_ = nullptr;
    }
    Converter& operator=(Converter const&) = delete;
    Converter& operator=(Converter const&&) = delete;

    ~Converter(void);

    // static bool known_conversion(ColorSpace source, ColorSpace target);

    Image const& operator()(ImageAllocator const& source) const;
    Image const& operator()(Image const& source) const;
    void operator()(Image const& source, Image& target) const;

    ImageHeader convert_header(ImageHeader const& source) const;

    Image const& result(void) const {
        return ((converter_ != nullptr) and
                (converter_->target.data != nullptr) and
                (converter_->target.header.format != ColorSpace::INVALID))
                   ? converter_->target
                   : invalid_image_;
    }

    void reset(void) {
        if (converter_ != nullptr) {
            converter_->target.header.format = ColorSpace::INVALID;
        }
    }

    ColorSpace target_format(void) const {
        return converter_ ? converter_->target_format_
                          : tv::ColorSpace::INVALID;
    }

    ColorSpace source_format(void) const {
        return converter_ ? converter_->source_format_
                          : tv::ColorSpace::INVALID;
    }
};

class FrameConversions {
private:
    Image const* frame_{nullptr};

    using ProvidedFormats = std::vector<Converter>;
    ProvidedFormats provided_formats_;

    Converter* get_converter(tv::ColorSpace from, tv::ColorSpace to) {

        auto it =
            std::find_if(provided_formats_.begin(), provided_formats_.end(),
                         [&](Converter const& converter) {

                return (converter.source_format() == from) and
                       (converter.target_format() == to);
            });

        if (it == provided_formats_.end()) {
            provided_formats_.emplace_back(from, to);
            it = --provided_formats_.end();
        }

        return &(*it);
    }

public:
    FrameConversions(void) noexcept(noexcept(std::vector<Converter>())) {}

    void set_frame(Image const& image) {
        frame_ = &image;
        for (auto& converter : provided_formats_) {
            // signal conversion necessary, see get_frame
            converter.reset();
        }
    }

    void get_frame(Image& image, tv::ColorSpace format) {
        assert(frame_ and frame_->header.format != ColorSpace::INVALID);

        // If the requested format is the same as provided by the camera,
        // image_.
        if (format == frame_->header.format) {
            image = *frame_;
            return;
        }

        // Else, check if a converter for the requested format already is
        // instantiated. If not, insert a new one. Else, check if it
        // contains a
        // valid result with the same timestamp as image_. If, it has been
        // run
        // already, just return the result. Else, run the converter.

        auto converter = get_converter(frame_->header.format, format);
        if (converter) {
            image = converter->result();

            if (image.header.format == tv::ColorSpace::INVALID or
                image.header.timestamp != frame_->header.timestamp) {

                // conversion and flat copy
                assert(frame_->data);
                image = (*converter)(*frame_);
            }
        }

        assert(image.header.format != tv::ColorSpace::INVALID);
    }

    tv::ImageHeader get_header(tv::ColorSpace format) {

        if (format == frame_->header.format) {
            return frame_->header;
        }

        auto converter = get_converter(frame_->header.format, format);
        if (not converter) {
            LogError("CAMERACONTROL", "Can't get header for format ", format,
                     " (baseformat: ", frame_->header.format, ")");
            return ImageHeader();
        }

        return converter->convert_header(frame_->header);
    }
};
}
#endif
