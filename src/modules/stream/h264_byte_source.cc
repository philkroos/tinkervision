/// \file h264_byte_source.cc
/// \author philipp.kroos@fh-bielefeld.de
/// \date 2015
///
/// \brief Part of the Tinkervision-Stream module.
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

#include "h264_byte_source.hh"

tv::H264ByteSource* tv::H264ByteSource::createNew(
    UsageEnvironment& env, tv::ExecutionContext& context) {

    return new tv::H264ByteSource{env, context};
}

EventTriggerId tv::H264ByteSource::eventTriggerId = 0;

unsigned tv::H264ByteSource::referenceCount = 0;

tv::H264ByteSource::H264ByteSource(UsageEnvironment& env,
                                   tv::ExecutionContext& context)
    : FramedSource{env}, context_(context) {

    if (referenceCount == 0) {
    }

    ++referenceCount;

    if (eventTriggerId == 0) {
        eventTriggerId =
            envir().taskScheduler().createEventTrigger(deliverFrame0);
    }
}

tv::H264ByteSource::~H264ByteSource(void) {
    --referenceCount;

    if (not referenceCount) {
    }

    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    eventTriggerId = 0;
}

void tv::H264ByteSource::deliverFrame0(void* clientData) {
    ((tv::H264ByteSource*)clientData)->deliverFrame();
}

void tv::H264ByteSource::doGetNextFrame() {

    while (not context_.quit and nals_.empty()) {
        context_.encoder.get_nals(nals_);

        gettimeofday(&currentTime, NULL);
    }

    if (not context_.quit and not nals_.empty()) {
        deliverFrame();
    }
}

void tv::H264ByteSource::deliverFrame() {
    if (!isCurrentlyAwaitingData() or context_.quit) {
        return;
    } else if (nals_.empty()) {
        return;
    }

    auto nal = nals_.front();
    nals_.pop();

    auto offset = int{0};

    if (nal.i_payload >= 4 && nal.p_payload[0] == 0 && nal.p_payload[1] == 0 &&
        nal.p_payload[2] == 0 && nal.p_payload[3] == 1) {

        offset = 4;
    } else {
        if (nal.i_payload >= 3 && nal.p_payload[0] == 0 &&
            nal.p_payload[1] == 0 && nal.p_payload[2] == 1) {

            offset = 3;
        }
    }

    if (static_cast<unsigned>(nal.i_payload - offset) > fMaxSize) {
        fFrameSize = fMaxSize;
        fNumTruncatedBytes = nal.i_payload - offset - fMaxSize;
    } else {
        fFrameSize = nal.i_payload - offset;
    }

    fPresentationTime = currentTime;
    memmove(fTo, nal.p_payload + offset, fFrameSize);

    FramedSource::afterGetting(this);
}
