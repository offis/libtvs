/*
 * Copyright (c) 2017 OFFIS Institute for Information Technology
 *                          Oldenburg, Germany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file   timed_reader.cpp
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed value streams for tracing (reader implementation)
 * \see    timed_reader.h, timed_stream.h
 */

#include "tvs/tracing/timed_reader_base.h"
#include "tvs/tracing/timed_stream_base.h"

#include "tvs/utils/assert.h"
#include "tvs/utils/report.h"

#include "tvs/tracing/report_msgs.h"

namespace tracing {

timed_reader_base::timed_reader_base(const char* name)
  : timed_object(name)
  , stream_()
  , listener_()
  , listen_mode_(timed_listener_if::NOTIFY_NONE)
{}

timed_reader_base::~timed_reader_base()
{
  detach();
}

void
timed_reader_base::attach(const char* name)
{
  auto str = host::lookup(name);
  attach(*str);
}

void
timed_reader_base::attach(timed_stream_base& stream)
{
  if (stream_) {
    SYSX_REPORT_ERROR(report::stream_attach) % stream.name()
      << "reader '" << name()
      << "' "
         "is already attached to stream '"
      << stream_->name() << "'";
    return;
  }

  stream_ = &stream;
  stream_->attach(*this);
}

void
timed_reader_base::detach()
{
  if (!stream_)
    return;
  stream_->detach(*this);
  stream_ = nullptr;
}

timed_reader_base::listener_mode
timed_reader_base::listen(timed_listener_if& listener, listener_mode mode)
{
  listener_mode ret = listen_mode_;
  listen_mode_ = mode;

  // discard registered listener
  if (mode == timed_listener_if::NOTIFY_NONE) {
    listener_ = nullptr;
    return ret;
  }

  // register listener
  if (!listener_)
    listener_ = &listener;

  // updated mode for registered listener
  if (listener_ == &listener)
    return ret;

  SYSX_ASSERT(false && "(Different) listener already attached!");

  return ret; // avoid compile error
}

} // namespace tracing

/* Taf!
 */
