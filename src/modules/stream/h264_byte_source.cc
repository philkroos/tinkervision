#include "h264_byte_source.hh"

tfv::H264ByteSource* tfv::H264ByteSource::createNew(
    UsageEnvironment& env, tfv::ExecutionContext& context) {

    return new tfv::H264ByteSource{env, context};
}

EventTriggerId tfv::H264ByteSource::eventTriggerId = 0;

unsigned tfv::H264ByteSource::referenceCount = 0;

tfv::H264ByteSource::H264ByteSource(UsageEnvironment& env,
                                    tfv::ExecutionContext& context)
    : FramedSource{env}, context_(context) {

    if (referenceCount == 0) {
    }

    ++referenceCount;

    if (eventTriggerId == 0) {
        eventTriggerId =
            envir().taskScheduler().createEventTrigger(deliverFrame0);
    }
}

tfv::H264ByteSource::~H264ByteSource(void) {
    --referenceCount;

    if (not referenceCount) {
    }

    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    eventTriggerId = 0;
}

void tfv::H264ByteSource::deliverFrame0(void* clientData) {
    ((tfv::H264ByteSource*)clientData)->deliverFrame();
}

void tfv::H264ByteSource::doGetNextFrame() {

    while (not context_.quit and nals_.empty()) {
        context_.encoder.get_nals(nals_);

        gettimeofday(&currentTime, NULL);
    }

    if (not context_.quit and not nals_.empty()) {
        deliverFrame();
    }
}

void tfv::H264ByteSource::deliverFrame() {
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
