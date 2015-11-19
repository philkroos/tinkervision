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

#ifndef H264_ENCODER_H
#define H264_ENCODER_H

#include <queue>   // encoder io
#include <vector>  // encoder io
#include <mutex>

#include "tinkervision/tinkervision_defines.h"

extern "C" {
#include <x264.h>
}

namespace x264 {
using Encoder = struct x264_t;
using Parameter = x264_param_t;
using Picture = x264_picture_t;
using Nal = x264_nal_t;
}

namespace tv {

class H264Encoder {

public:
    ~H264Encoder(void);

    void initialize(std::size_t framewidth, std::size_t frameheight,
                    std::size_t fps);

    /**
     * Add a frame to be encoded.
     */
    void add_frame(uint8_t const* data);

    /**
     * Push all encoded nal-unites into output and clear the
     * internal queue.
     */
    void get_nals(std::queue<x264::Nal>& output);

    /**
     * Clear the internal queue.
     */
    void discard_all(void);

    /**
     * Return the size of the internal (output) queue
     */
    size_t nals_encoded(void) const;

private:
    x264::Encoder* encoder_{nullptr};
    x264::Parameter parameter_;
    x264::Picture picture_;
    std::vector<x264::Nal> nals_;
    std::size_t framesize_{0};
    std::mutex mutable io_lock_;
    bool initialized_ = false;
};
}

#endif /* H264_ENCODER_H */
