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

#include <chrono>

#include "stream.hh"
#include "exceptions.hh"

IMPLEMENT_EMPTY_INTERFACE(Stream)

tfv::Stream::~Stream(void) {
    killswitch_ = 1;
    context_.quit = true;

    std::chrono::milliseconds span(100);
    while (streamer_.wait_for(span) == std::future_status::timeout)
        ;
    streamer_.get();
}

tfv::Stream::Stream(TFV_Int module_id, Module::Tag tags)
    : Executable(module_id, "Stream", tags), context_(ExecutionContext::get()) {

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

void tfv::Stream::execute(tfv::Image const& image) {
    if (not subsession_) {

        context_.encoder.initialize(image.width, image.height, 10);  // FPS!

        subsession_ =
            tfv::H264MediaSession::createNew(*usage_environment_, context_);

        session_->addSubsession(subsession_);
        rtsp_server_->addServerMediaSession(session_);

        streamer_ = std::async(std::launch::async, [this](void) {
            task_scheduler_->doEventLoop(&killswitch_);
        });

        std::cout << "Play the stream using " << rtsp_server_->rtspURL(session_)
                  << std::endl;
    }

    // if no one is watching anyway, discard old data
    if (context_.encoder.nals_encoded() > 10) {  // arbitrary
        context_.encoder.discard_all();
    }

    context_.encoder.add_frame(image.data);
}
