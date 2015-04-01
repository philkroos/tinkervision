/*
Tinkervision - Vision Library for https://github.com/Tinkerforge/red-brick
Copyright (C) 2014-2015 philipp.kroos@fh-bielefeld.de

Using the helpful example from
http://stackoverflow.com/questions/19427576/live555-x264-stream-live-source-based-on-testondemandrtspserver

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

#ifndef H264MEDIASESSION_H
#define H264MEDIASESSION_H

#include <liveMedia.hh>
#include <OnDemandServerMediaSubsession.hh>

#include "h264_byte_source.hh"
#include "h264_encoder.hh"

namespace tfv {
class H264MediaSession : public OnDemandServerMediaSubsession {
public:
    static H264MediaSession* createNew(UsageEnvironment& env,
                                       tfv::H264Encoder& encoder);
    void check_for_aux_sdp_line(void);
    void after_playing_dummy(void);

protected:
    H264MediaSession(UsageEnvironment& env, tfv::H264Encoder& encoder);
    virtual ~H264MediaSession(void);
    void setDoneFlag() { done_flag_ = ~0; }

protected:
    virtual char const* getAuxSDPLine(RTPSink* rtpSink,
                                      FramedSource* inputSource);
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
                                                unsigned& estBitrate);

    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                      unsigned char rtpPayloadTypeIfDynamic,
                                      FramedSource* inputSource);

private:
    static const bool REUSE_FIRST_SOURCE = true;
    char* aux_SDP_line_;
    char done_flag_;
    RTPSink* dummy_sink_;

    H264Encoder& encoder_;
};
}

#endif
