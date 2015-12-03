/// \file stream.hh
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Declaration of the module \c Stream.
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

#ifndef STREAM_H
#define STREAM_H

#include "module.hh"

#include <future>  // async

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

#include "execution_context.hh"
#include "h264_media_session.hh"
#include "h264_encoder.hh"

namespace tv {

class Stream : public Module {
public:
    Stream(Environment const& envir);

    ~Stream(void) override;

protected:
    void execute(tv::ImageHeader const& header, tv::ImageData const* data,
                 tv::ImageHeader const&, tv::ImageData*) override final;

    ColorSpace input_format(void) const override { return ColorSpace::YV12; }

    bool has_result(void) const override final { return false; }
    bool outputs_image(void) const override final { return false; }
    bool produces_result(void) const override final { return false; }

    void stop(void) override final;

private:
    TaskScheduler* task_scheduler_ = nullptr;
    BasicUsageEnvironment* usage_environment_ = nullptr;
    RTSPServer* rtsp_server_ = nullptr;
    ServerMediaSession* session_ = nullptr;

    int port_ = 8554;
    const char* streamname_{"tv"};
    const char* streamtypename_{"H.264 stream"};
    std::string url_{""};  ///< URL of the stream:
    /// rtsp://<ip>:port_/streamname_. Retrievable from
    /// the read-only string parameter url once streaming.

    H264MediaSession* subsession_ = nullptr;
    ExecutionContext& context_;

    using AsyncTask = std::future<void>;
    AsyncTask streamer_;

    char killswitch_ = 0;  ///< signal live555 to stop the event loop

    void setup(void);
};
}

DECLARE_VISION_MODULE(Stream)

#endif /* STREAM_H */
