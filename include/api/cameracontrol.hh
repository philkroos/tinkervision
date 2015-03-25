/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014 philipp.kroos@fh-bielefeld.de

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

#ifndef CAMERACONTROL_H
#define CAMERACONTROL_H

#include <sys/stat.h>  // stat, for _device_exists()
#include <string>
#include <mutex>

#include "opencv_camera.hh"
#include "v4l2_camera.hh"
#include "image.hh"

namespace tfv {

class CameraControl {
public:
    CameraControl(void) = default;
    ~CameraControl(void);

    CameraControl(CameraControl const&) = delete;
    CameraControl& operator=(CameraControl const&) = delete;

    /**
     * Check if (any) cameradevice is available.
     * If no device is open already, seeks for a possible device, opens it,
     * closes it.  Does not increase the usercount.  Does not affect visible
     * state.
     * \return
     *         - True if a device is already open or can be opened.
     *         - False if not and any of the steps above fails.
     */
    bool is_available(void);

    /**
     * Check if (any) cameradevice is available, acquire it if necessary.
     * If no device is open already, seeks for a possible device, opens it,
     * increases the usercount.  No effects on visible state if opening fails.
     * \return
     *         - True if a device is open.
     *         - False if not and any of the steps above fails.
     */
    bool acquire(void);

    /**
     * Same as calling acquire(), add_user(max(0, user-1)). No effect if user is
     * 0.
     * \parm[in] user Reserve the camera for that many users.
     * \return
     *         - True if a device is open.
     *         - False if not and any of the steps above fails.
     */
    bool acquire(size_t user);

    /**
     * Check if a camera is open. No effects on visible state.
     * \return True if a device is open.
     */
    bool is_open(void);

    /**
     * Reduce the usercount by one, possibly release the camera device.
     * No effects if no user registered.
     */
    void release();

    /**
     * Get the number of users currently registered. No effect on internal or
     * visible state.
     * \return usercount_
     */
    size_t usercount(void) const {

        // usercount_ never becomes negative
        return static_cast<size_t>(usercount_);
    }

    /**
     * Releases the acquired device, if any, without resetting it's usercount.
     * This is usefull to pause framegrabbing and returning to the known state
     * later.  Does not reduce the usercounter.
     */
    void stop_camera(void);

    /**
     * Retrieves the frame properties from an opened device. No effect on
     * visible state.
     * \param[out] height (visible) image height, i.e. in pixel
     * \param[out] width (visible) image width, i.e. in pixel
     * \param[out] bytesize total image width, i.e. in byte
     * \return False if no camera is opened or retrieving the values fails.
     */
    bool get_properties(size_t& height, size_t& width, size_t& bytesize);

    /**
     * Grab a frame if a camera is available.
     * If the camera had been stopped before, tries to open it again; requests a
     * frame.
     * \parm[out] tfv::Image for which, on success, all members are valid.
     * \return True if image acquisition succeeded.
     */
    bool get_frame(tfv::Image& Image);

    /**
     * Add a number to the internal usercounter.
     */
    void add_user(size_t count) { usercount_ += count; }

private:
    Camera* camera_ = nullptr;

    using FallbackImage = struct _ {
        Image image = {};
        bool active = false;
    };

    FallbackImage fallback;

    int usercount_ = 0;
    bool stopped_ = false;

    inline bool _device_exists(int number) const {
        struct stat buffer;
        return (stat(std::string("/dev/video" + std::to_string(number)).c_str(),
                     &buffer) == 0);
    }

    std::mutex camera_mutex_;  //< This locks access to the private methods

    bool _open_device();
    void _close_device();
};
};

#endif /* CAMERACONTROL_H */
