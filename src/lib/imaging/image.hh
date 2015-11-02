/// \file image.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of the classes \c Image, \c ImageHeader and \c
/// ImageAllocator.
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

#ifndef IMAGING_H
#define IMAGING_H

#include <chrono>  // timestamp
#include <string>

#include "tinkervision_defines.h"

namespace tv {

/// Supported image formats. The value range per entry is 0-255 for each format,
/// but the number of bytes per pixel differs.
/// - NONE: Can be used by modules that don't want to process images.
/// - INVALID: This would be an error.
/// - YUYV: packed Y'CbCr data, i.e. Y,V and U are stored in the same
/// datablock, structured like:
/// Y00 U00 Y01 V00 Y02 U01 Y03 V01 Y04 ...
/// Y10 U10 Y11 V10 Y12 U11 Y13 V11 Y14 ...
/// ...
/// For each two Y-values there is one U and one V and one pixel is represented
/// by two bytes. So the number of bytes per row equals width*2.
/// This equals YUV422.
///
/// - YV12: planar Y'CbCr data, i.e. Y,U and V are stored in consecutive
/// datablocks, structured like:
/// Y00 Y01 Y02 ...
/// Y10 Y11 Y12 ...
/// ...
/// U00 U01 U02 ...
/// ...
/// V00 V01 V02 ...
/// ...
/// where the count(U) = count(V) = count(Y)/4, i.e. count(Y) = width * height
/// and the resolution of U and V is halfed vertically and horizontally.
/// - RGB888: One pixel p = 3 byte where p(1) = R, p(2) = G, p(3) = B
/// So the size of one image is height * width * 3.
/// - BGR888: The same as RGB888 but reordered.
enum class ColorSpace : char {
    /// \todo Is None still in use?
    NONE,
    INVALID,
    YUYV,
    /*YVYU,*/
    YV12,
    BGR888,
    RGB888,
    GRAY
};

using Clock = std::chrono::steady_clock;  ///< Clock used for image timestamps.
using Timestamp = Clock::time_point;      ///< Convenience typedef.
using ImageData = TV_ImageData;           ///< Convenience typedef.

/// Header for a frame.
struct ImageHeader {
    uint16_t width = 0;                       ///< Framewidth.
    uint16_t height = 0;                      ///< Frameheight.
    size_t bytesize = 0;                      ///< Size in bytes.
    Timestamp timestamp;                      ///< When grabbed.
    ColorSpace format = ColorSpace::INVALID;  ///< Frame colorspace.

    operator bool(void) const {
        return width > 0 and height > 0 and bytesize > 0 and
               format != tv::ColorSpace::INVALID and
               format != tv::ColorSpace::NONE;
    }
};

bool operator==(ImageHeader const& lhs, ImageHeader const& rhs);
bool operator!=(ImageHeader const& lhs, ImageHeader const& rhs);

/// Image consists of a header and a data pointer.
/// The data pointer may be initialized with a new block, but it can also point
/// to existing data. This is handled in ImageAllocator.
/// \note Never use this structure directly. Always use the ImageAllocator.
struct Image {
    ImageHeader header;         ///< Describing the frame.
    ImageData* data = nullptr;  ///< Frame data.
};

/// Utility class to manage allocation and sharing of frame data
/// blocks across different parts of the library.  The semantic means
/// of this class is described with the methods allocate(),
/// set_from_image() and copy_data().  To get access to the contained
/// image, use image().  Allocated data is automatically destroyed
/// during destruction or, where necessary, during assignment of
/// different image data.
class ImageAllocator {
private:
    Image image_;                    ///< The managed frame.
    size_t image_init_bytesize_{0};  ///< Initial bytesize during allocation.
    bool using_foreign_data_{
        false};  ///< True if image_ points to other image's data

    std::string const id_;  ///< Merely of debugging value.
    size_t max_size_{0};  ///< Currently unused, optional size limit if known at
    /// initialization.

    /// Internal helper.
    void _free_image(void);

public:
    /// Standard c'tor.
    explicit ImageAllocator(std::string const& id);

    /// Currently of no value because max_size_ is ignored.
    ImageAllocator(std::string const& id, size_t known_max_size);

    ImageAllocator(ImageAllocator const&) = delete;
    ImageAllocator& operator=(ImageAllocator const&) = delete;

    /// D'tor
    ~ImageAllocator(void);

    /// Allocate a new Image.
    /// \param[in] width Framewidth.
    /// \param[in] width Frameheight.
    /// \param[in] bytesize Framesize in bytes.
    /// \param[in] format Frame color space.
    /// \param[in] foreign_data If true, no new block of memory will be
    /// allocated.
    bool allocate(uint16_t width, uint16_t height, size_t bytesize,
                  ColorSpace format, bool foreign_data);

    /// Convenience method forwarding to the more explicit allocate().
    /// \param[in] header Parameters of the frame to be allocated.
    /// \param[in] foreign_data If true, no memory will be allocated.
    bool allocate(ImageHeader const& header, bool foreign_data) {
        // \todo Allow passing the same header. Possible results if the same
        // header is passed are not thought through yet.
        assert(&header != &image_.header);

        /// It is currently not allowed to pass the header used by the wrapped
        /// image to this method
        if (&header == &image_.header) {
            return false;
        }
        return allocate(header.width, header.height, header.bytesize,
                        header.format, foreign_data);
    }

    /// Create a reference to another image.
    void set_from_image(Image const& image);

    /// Create a deep copy of image data.
    void copy_data(ImageData const* data, size_t size);

    /// Access the managed image.
    /// \return image_.
    Image& image(void) { return image_; }

    /// Access the header of the managed image.
    /// \return image_.header.
    ImageHeader const& header(void) { return image_.header; }

    /// Access the managed image in a const context.
    /// \return const image_.
    Image const& operator()(void) const { return image_; }
};
}

#endif
