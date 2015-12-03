/// \file camera.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Declaration of Camera.
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

#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>

#include "tinkervision_defines.h"
#include "image.hh"

namespace tv {

/// Abstract camera interface used by Tinkervision.
class Camera {
public:
    virtual ~Camera(void) = default;

    void stop(void);
    bool get_frame(Image& frame);
    bool get_properties(uint16_t& height, uint16_t& width,
                        size_t& framebytesize);

    /// Get the loaded device's id.
    /// \return camera_id_.
    uint8_t id(void) const { return camera_id_; }

    /// This is a shortcut for open(0, 0).
    bool open(void);

    /// Open the camera with the specified framesize.
    /// If the framesize does not matter, pass 0,0.
    /// \param[in] width The requested framewidth. Pass 0 if the framesize
    /// should be selected automatically.
    /// \param[in] height The requested height. Ignored if width is 0.
    /// \return True if the camera is open. If width was specified as 0,
    /// retrieve the actual framesize with get_properties().
    bool open(uint16_t width, uint16_t height);

    ImageHeader frame_header(void) const { return image_.header; }

    /// Just check if the device is open, do not change the current state.
    virtual bool is_open(void) const = 0;
    virtual ColorSpace image_format(void) const = 0;

protected:
    explicit Camera(uint8_t camera_id);
    uint8_t camera_id_;

    virtual bool open_device(void) = 0;
    virtual bool open_device(uint16_t width, uint16_t height) = 0;
    virtual bool retrieve_frame(tv::ImageData** data) = 0;
    virtual void retrieve_properties(uint16_t& width, uint16_t& height,
                                     size_t& framebytesize) = 0;
    virtual void close(void) = 0;

private:
    bool active_{true};

    Image image_{};  ///< Image container, data filled by subclass
};
}

#endif /* CAMERA_H */
