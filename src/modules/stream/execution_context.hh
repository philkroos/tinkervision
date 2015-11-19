/// \file execution_context.hh
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

#ifndef EXECUTION_CONTEXT_H
#define EXECUTION_CONTEXT_H

#include "h264_encoder.hh"

namespace tv {

struct ExecutionContext {
    H264Encoder encoder;
    bool quit = false;

    static ExecutionContext& get(void) {
        if (not ExecutionContext::singleton_) {
            singleton_ = new ExecutionContext{};
        }
        return *ExecutionContext::singleton_;
    }

    ~ExecutionContext(void) {
        if (ExecutionContext::singleton_) {
            delete ExecutionContext::singleton_;
        }
    }

private:
    ExecutionContext(void) = default;
    static ExecutionContext* singleton_;
};
}

#endif
