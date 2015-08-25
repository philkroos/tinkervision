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

#include "h264_encoder.hh"

namespace x264 {
// static auto set_paramater_preset{x264_param_default_preset};
static auto apply_parameter_profile = x264_param_apply_profile;
static auto allocate_picture = x264_picture_alloc;
static auto open_encoder = x264_encoder_open;

static auto YV12 = X264_CSP_I420;
static auto PICTURE_TYPE_AUTOMATIC = X264_TYPE_AUTO;
static auto RC_CRF = X264_RC_CRF;

// static auto WRITE_NAL_SIZE = 0;
static auto WRITE_STARTCODE = 1;
}

tfv::H264Encoder::~H264Encoder(void) {
    if (initialized_) {
        x264_encoder_close(encoder_);
        x264_picture_clean(&picture_);
    }
}

void tfv::H264Encoder::initialize(std::size_t framewidth,
                                  std::size_t frameheight, std::size_t fps) {

    framesize_ = framewidth * frameheight;
    x264_param_default_preset(&parameter_, "ultrafast", "zerolatency");

    // Header x264 in http://git.videolan.org/?p=x264.git;a=summary
    parameter_.i_log_level = X264_LOG_NONE;
    parameter_.i_threads = 1;
    parameter_.i_width = framewidth;
    parameter_.i_height = frameheight;
    parameter_.i_fps_num = fps;
    parameter_.i_fps_den = 1;
    // at most 25 intra key frames (i.e., clients get opportunity to jump
    // into
    // stream at least with every 25th frame)
    parameter_.i_keyint_max = 25;
    parameter_.b_intra_refresh = 1;
    parameter_.rc.i_rc_method = x264::RC_CRF;
    parameter_.rc.i_vbv_buffer_size = 1000000;
    parameter_.rc.i_vbv_max_bitrate = 90000;
    parameter_.rc.f_rf_constant = 25;
    parameter_.rc.f_rf_constant_max = 35;
    parameter_.i_sps_id = 7;

    // put the stream header before every I-(key)frame
    parameter_.b_repeat_headers = 1;

    // Either a mpeg-startcode (0x00,0x00,0x00,0x01 or 0X00,0x00,0x01) or
    // the
    // size of the nal (in 4 bytes) will be written in front of each nal.
    parameter_.b_annexb = x264::WRITE_STARTCODE;

    x264::apply_parameter_profile(&parameter_, "baseline");

    encoder_ = x264::open_encoder(&parameter_);
    x264::allocate_picture(&picture_, x264::YV12, parameter_.i_width,
                           parameter_.i_height);
    picture_.i_type = x264::PICTURE_TYPE_AUTOMATIC;
    picture_.img.i_csp = x264::YV12;

    initialized_ = true;
}

void tfv::H264Encoder::add_frame(TFV_ImageData* data) {
    x264::Nal* nals{};
    auto nal_count = int{0};

    picture_.img.plane[0] = data;                                    // y
    picture_.img.plane[2] = data + framesize_;                       // v
    picture_.img.plane[1] = picture_.img.plane[2] + framesize_ / 4;  // u

    x264_picture_t picture_out;
    auto encoded = x264_encoder_encode(encoder_, &nals, &nal_count, &picture_,
                                       &picture_out);

    if (encoded > 0) {
        std::lock_guard<std::mutex> lock(io_lock_);
        for (auto i = 0; i < nal_count; ++i) {
            nals_.push_back(nals[i]);
        }
    }
}

void tfv::H264Encoder::get_nals(std::queue<x264::Nal>& output) {
    std::lock_guard<std::mutex> lock(io_lock_);
    for (auto const& nal : nals_) {
        output.push(nal);
    }
    nals_.clear();
}

void tfv::H264Encoder::discard_all(void) {
    std::lock_guard<std::mutex> lock(io_lock_);
    nals_.clear();
}

size_t tfv::H264Encoder::nals_encoded(void) const {
    std::lock_guard<std::mutex> lock(io_lock_);
    return nals_.size();
}
