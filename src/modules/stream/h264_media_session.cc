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

#include "h264_media_session.hh"

#include <iostream>

tfv::H264MediaSession* tfv::H264MediaSession::createNew(
    UsageEnvironment& env, tfv::ExecutionContext& context) {

    return new tfv::H264MediaSession(env, context);
}

tfv::H264MediaSession::H264MediaSession(UsageEnvironment& env,
                                        tfv::ExecutionContext& context)
    : OnDemandServerMediaSubsession(env, REUSE_FIRST_SOURCE),
      aux_SDP_line_(NULL),
      done_flag_(0),
      dummy_sink_(NULL),
      context_(context) {}

tfv::H264MediaSession::~H264MediaSession(void) { delete[] aux_SDP_line_; }

static void afterPlayingDummy(void* clientData) {
    tfv::H264MediaSession* session = (tfv::H264MediaSession*)clientData;
    session->after_playing_dummy();
}

void tfv::H264MediaSession::after_playing_dummy() {
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) {
    tfv::H264MediaSession* session = (tfv::H264MediaSession*)clientData;
    session->check_for_aux_sdp_line();
}

void tfv::H264MediaSession::check_for_aux_sdp_line() {
    char const* dasl;
    if (aux_SDP_line_ != NULL) {
        setDoneFlag();

    } else if (dummy_sink_ != NULL &&
               (dasl = dummy_sink_->auxSDPLine()) != NULL) {

        aux_SDP_line_ = strDup(dasl);
        dummy_sink_ = NULL;
        setDoneFlag();

    } else {
        // std::cout << "checking again later" << std::endl;
        int uSecsDelay = 100000;
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            uSecsDelay, (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const* tfv::H264MediaSession::getAuxSDPLine(RTPSink* rtpSink,
                                                 FramedSource* inputSource) {
    if (aux_SDP_line_ == NULL) {
        if (dummy_sink_ == NULL) {
            // std::cout << "Start playing dummy sink" << std::endl;
            dummy_sink_ = rtpSink;
            dummy_sink_->startPlaying(*inputSource, afterPlayingDummy, this);
            checkForAuxSDPLine(this);
        }

        envir().taskScheduler().doEventLoop(&done_flag_);
    }
    return aux_SDP_line_;
}

FramedSource* tfv::H264MediaSession::createNewStreamSource(
    unsigned clientSessionID, unsigned& estBitRate) {

    /// \todo Adjust this with encoder settings
    estBitRate = 90000;

    OutPacketBuffer::maxSize = 200000;

    tfv::H264ByteSource* source =
        tfv::H264ByteSource::createNew(envir(), context_);

    return H264VideoStreamDiscreteFramer::createNew(envir(), source);
}

RTPSink* tfv::H264MediaSession::createNewRTPSink(
    Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* inputSource) {
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock,
                                       rtpPayloadTypeIfDynamic);
}
