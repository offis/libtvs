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
 * \file   timed_stream_base.cpp
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed stream base class implementation
 * \see    timed_object.h
 */

#include <algorithm>

#include "tvs/tracing/timed_reader_base.h"
#include "tvs/tracing/timed_stream_base.h"
#include "tvs/tracing/timed_writer_base.h"

#include "report_msgs.h"

namespace tracing {

/* ------------------------- timed_stream_base ------------------------- */

timed_stream_base::timed_stream_base(const char* nm)
  : timed_object(nm)
  , writer_()
  , readers_()
{}

timed_stream_base::~timed_stream_base()
{
  while (readers_.begin() != readers_.end())
    readers_.front()->detach();

  if (writer_)
    writer_->detach();
}

void
timed_stream_base::attach(timed_writer_base& writer)
{
  if (writer_) {
    SYSX_REPORT_ERROR(report::stream_attach) % name()
      << "writer already attached";
    return;
  }
  if (!do_type_check(writer)) {
    SYSX_REPORT_ERROR(report::stream_attach) % name()
      << "incompatible writer"; /// FIXME: add type information
    return;
  }
  writer_ = &writer;
}

void
timed_stream_base::detach(timed_writer_base& writer)
{
  if (&writer == writer_)
    writer_ = nullptr;
}

void
timed_stream_base::attach(timed_reader_base& reader)
{
  typedef std::vector<timed_reader_base*>::iterator reader_iterator;
  reader_iterator it = std::find(readers_.begin(), readers_.end(), &reader);
  if (it != readers_.end()) {
    SYSX_REPORT_ERROR(report::stream_attach) % name()
      << "reader '" << reader.name() << "' already attached";
    return;
  }
  if (!do_type_check(reader)) {
    SYSX_REPORT_ERROR(report::stream_attach) % name()
      << "incompatible reader '" << reader.name() << "'";
    /// FIXME: add type information
    return;
  }
  readers_.push_back(&reader);
}

void
timed_stream_base::detach(timed_reader_base& reader)
{
  typedef std::vector<timed_reader_base*>::iterator reader_iterator;
  reader_iterator it = std::remove(readers_.begin(), readers_.end(), &reader);
  readers_.erase(it, readers_.end());
}

timed_stream_base::duration_type
timed_stream_base::do_commit(duration_type until)
{
  typedef std::vector<timed_reader_base*>::iterator reader_iterator;

  if (until == timed_duration::zero_time)
    until = duration();

  reader_iterator begin = readers_.begin(), end = readers_.end();

  do_pre_commit_reader(until);

  // no readers
  if (begin == end) {
    do_clear();
    return until;
  }

  while (end - begin > 1) {
    do_commit_reader(**(begin++), until);
  }

  do_commit_reader(**begin, until, /* last = */ true);
  return until;
}

} // namespace tracing

/* Taf!
 */
