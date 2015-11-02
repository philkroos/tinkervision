/// \file stream.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Implementation of the module \c Stream.
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

#include "stream.hh"

#include <chrono>

#include "tinkervision/exceptions.hh"

DEFINE_VISION_MODULE(Stream)

tv::Stream::~Stream(void) {
    killswitch_ = 1;
    context_.quit = true;

    std::chrono::milliseconds span(100);
    while (streamer_.wait_for(span) == std::future_status::timeout)
        ;
    streamer_.get();
}

tv::Stream::Stream() : Module("Stream"), context_(ExecutionContext::get()) {

    task_scheduler_ = BasicTaskScheduler::createNew();

    usage_environment_ = BasicUsageEnvironment::createNew(*task_scheduler_);

    rtsp_server_ = RTSPServer::createNew(*usage_environment_, port_, nullptr);

    if (not rtsp_server_) {
        throw ConstructionException("Stream",
                                    usage_environment_->getResultMsg());
    }

    session_ = ServerMediaSession::createNew(*usage_environment_, streamname_,
                                             streamname_, streamtypename_);
}

void tv::Stream::execute(tv::ImageHeader const& header,
                         tv::ImageData const* data, tv::ImageHeader const&,
                         tv::ImageData*) {
    if (not subsession_) {

        context_.encoder.initialize(header.width, header.height, 10);  // FPS!

        subsession_ =
            tv::H264MediaSession::createNew(*usage_environment_, context_);

        session_->addSubsession(subsession_);
        rtsp_server_->addServerMediaSession(session_);

        streamer_ = std::async(std::launch::async, [this](void) {
            task_scheduler_->doEventLoop(&killswitch_);
        });

        // std::cout << "Play the stream using " <<
        // rtsp_server_->rtspURL(session_)
        //           << std::endl;
    }

    // if no one is watching anyway, discard old data
    if (context_.encoder.nals_encoded() > 10) {  // arbitrary 10
        context_.encoder.discard_all();
    }

    /// \todo check for constant frame dimensions
    context_.encoder.add_frame(data);
}
