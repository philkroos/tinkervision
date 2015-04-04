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

#include "stream.hh"
#include "exceptions.hh"

tfv::Stream::~Stream(void) {
    std::cout << "Destroying module Stream" << std::endl;
    killswitch_ = 1;
    (void)streamer_.get();
}

tfv::Stream::Stream(TFV_Int module_id) : Executable(module_id, "Stream") {

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
        std::cout << std::endl << "Initializing encoder" << std::endl;
        encoder_.initialize(image.width, image.height, 10);  // FPS!

        // ugly thing to share the encoder
        subsession_ =
            tfv::H264MediaSession::createNew(*usage_environment_, encoder_);

        session_->addSubsession(subsession_);
        rtsp_server_->addServerMediaSession(session_);

        auto url = std::string{rtsp_server_->rtspURL(session_)};

        std::cout << "Play the stream using " << url << std::endl;
        // asynchronous_execution(
        //     [this](void) { this->task_scheduler_->doEventLoop(); });
        streamer_ = std::async(std::launch::async, [this](void) {
            task_scheduler_->doEventLoop(&killswitch_);
        });
        std::cout << "After starting streamer" << std::endl;
    }

    // if no one is watching anyway, discard old data
    if (encoder_.nals_encoded() > 10) {  // arbitrary
        encoder_.discard_all();
    }
    encoder_.add_frame(image.data);
}

template <>
bool tfv::valid<tfv::Stream>(void) {
    return true;
}
