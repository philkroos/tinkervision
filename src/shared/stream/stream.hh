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

#ifndef STREAM_H
#define STREAM_H

#include "tv_module.hh"

#include <future>  // async

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

#include "execution_context.hh"
#include "h264_media_session.hh"
#include "h264_encoder.hh"

namespace tfv {

struct Stream : public TVModule {

    Stream(void);

    ~Stream(void) override;

    void execute(tfv::ImageData const* data, size_t width,
                 size_t height) override;

    ColorSpace expected_format(void) const override { return ColorSpace::YV12; }

private:
    TaskScheduler* task_scheduler_ = nullptr;
    BasicUsageEnvironment* usage_environment_ = nullptr;
    RTSPServer* rtsp_server_ = nullptr;
    ServerMediaSession* session_ = nullptr;

    int port_ = 8554;
    const char* streamname_{"tinkervision"};
    const char* streamtypename_{"H.264 stream"};

    H264MediaSession* subsession_ = nullptr;
    ExecutionContext& context_;

    using AsyncTask = std::future<void>;
    AsyncTask streamer_;

    char killswitch_ = 0;  ///< signal live555 to stop the event loop
};
}

DECLARE_VISION_MODULE(Stream)

#endif /* STREAM_H */
