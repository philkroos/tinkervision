/// \file cameracontrol.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2014-2015
///
/// \brief Declares CameraControl.
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

#ifndef CAMERACONTROL_H
#define CAMERACONTROL_H

#include <sys/stat.h>  // stat, for _device_exists()
#include <string>
#include <mutex>

#include "image.hh"
#include "convert.hh"
#include "camera.hh"

namespace tv {

class CameraControl {
public:
    CameraControl(void) noexcept;
    ~CameraControl(void);

    CameraControl(CameraControl const&) = delete;
    CameraControl& operator=(CameraControl const&) = delete;

    /// Check if (any) cameradevice is available.
    /// If no device is open already, seeks for a possible device, opens it,
    /// closes it.  Does not increase the usercount.  Does not affect visible
    /// state.
    /// \return
    ///         - True if a device is already open or can be opened.
    ///         - False if not and any of the steps above fails.
    bool is_available(void);

    /// Same as is_available for a specific camera.
    /// \param[in] id Device id.
    bool is_available(uint8_t id);

    /// Select a specific device as preferred camera. On acquire() and other
    /// methods needing to open a device, this one will always be preferred.
    /// \param[in] id Device id.
    /// \return is_available(id).
    bool prefer(uint8_t id);

    /// Switch to the given device, if possible. Calls prefer() first.
    /// \param[in] device Preferred device.
    /// \return true If device is available and the current is_open() state has
    /// not changed.
    bool switch_to_preferred(uint8_t device);

    /// Request a framesize to be set when initializing the camera.
    /// This will only work the camera is not active.
    /// \param[in] framewidth Width requested
    /// \param[in] framheight Height requested
    bool preselect_framesize(uint16_t framewidth, uint16_t frameheight);

    /// Check if (any) cameradevice is available, acquire it if necessary.
    /// If no device is open already, seeks for a possible device, opens it,
    /// increases the usercount.  No effects on visible state if opening fails.
    /// \return
    ///         - True if a device is open.
    ///         - False if not and any of the steps above fails.
    bool acquire(void);

    /// Same as calling acquire(), add_user(max(0, user-1)). No effect if user
    /// is 0.
    /// \param[in] user Reserve the camera for that many users.
    /// \return
    ///         - True if a device is open.
    ///         - False if not and any of the steps above fails.
    bool acquire(size_t user);

    /// Check if a camera is open. No effects on visible state.
    /// \return True if a device is open.
    bool is_open(void) const;

    /// Get the id of the open camera, if any.
    /// \return -1 if no camera is open, camera_->id() else.
    int16_t current_device(void) const;

    /// Reduce the usercount by one, possibly release the camera device.
    /// No effects if no user registered.
    void release();

    /// Get the number of users currently registered. No effect on internal or
    /// visible state.
    /// \return usercount_
    size_t usercount(void) const {

        // usercount_ never becomes negative
        return static_cast<size_t>(usercount_);
    }

    /// Releases the acquired device, if any, without resetting it's usercount.
    /// This is usefull to pause framegrabbing and returning to the known state
    /// later.  Also used to switch the selected device.
    void stop_camera(void);

    /// Releases the acquired device, if any, resetting it's usercount.
    void release_all(void);

    /// Retrieves the frame properties from an opened device. No effect on
    /// visible state.
    /// \param[out] height (visible) image height, i.e. in pixel
    /// \param[out] width (visible) image width, i.e. in pixel
    /// \param[out] bytesize total image width, i.e. in byte
    /// \return False if no camera is opened or retrieving the values fails.
    bool get_properties(uint16_t& height, uint16_t& width, size_t& bytesize);

    /// Retrieves the frame properties from an opened device. No effect on
    /// visible state.
    /// \param[out] height (visible) image height, i.e. in pixel
    /// \param[out] width (visible) image width, i.e. in pixel
    /// \return False if no camera is opened or retrieving the values fails.
    bool get_resolution(uint16_t& width, uint16_t& height);

    /// Grab a frame if a camera is available.  Sets image_.
    /// If the camera had been stopped before, tries to open it again; requests
    /// a
    /// frame.
    /// \param[out] image Set to the grabbed frame on success, else not touched.
    /// \return True if image acquisition succeeded.
    bool update_frame(Image& image);

    /// Add a number to the internal usercounter.
    void add_user(size_t count) { usercount_ += count; }

    Timestamp latest_frame_timestamp(void) const {
        return image_().header.timestamp;
    }

    /// \return prefered_device_ is not -1.
    bool device_preferred(void) const { return preferred_device_ >= 0; }

private:
    Camera* camera_{nullptr};
    size_t requested_width_{640};
    size_t requested_height_{480};

    int16_t preferred_device_{-1};  ///< If any device id is preferred, >= 0.

    ImageAllocator fallback_{"CC/Fallback"};  ///< Black frame
    ImageAllocator image_{"CC/Image"};        ///< Data exchanged with the Api

    int usercount_ = 0;
    bool stopped_ = false;

    std::mutex camera_mutex_;  //< This locks access to the private methods

    /// Open a device.
    bool _open_device(Camera** device);
    /// Open a specific device.
    bool _open_device(Camera** device, uint8_t id);

    /// Close a device.
    void _close_device(Camera** device);
    /// Test camera_, locking it.
    bool _test_device(void);
    bool _test_device(Camera** cam, uint8_t device);
    bool _init(void);
    bool _update_from_camera(void);
    bool _update_from_fallback(void);
};
}

#endif
