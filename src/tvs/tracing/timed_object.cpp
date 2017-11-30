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
 * \file   timed_object.cpp
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed streaming base classes
 * \see    timed_object.h
 */

#include "tvs/tracing/timed_object.h"
#include "tvs/tracing/object_host.h"
#include "tvs/tracing/timed_stream_base.h"

#include "report_msgs.h"

#include "tvs/utils/debug.h"
#include "tvs/utils/macros.h"

namespace tracing {


/* ----------------------------- sync --------------------------- */

time_type
sync()
{
  time_type max_time;
  host::for_each_stream_in_scope([&max_time](timed_stream_base* stream) {
    max_time = std::max(max_time, stream->end_time());
    return false;
  });

  sync(max_time);

  return max_time;
}

void
sync(time_type const& until)
{
  host::for_each_stream_in_scope([&until](timed_stream_base* stream) {
    stream->commit(until);
    return false;
  });
}

/* ----------------------------- timed_base --------------------------- */

timed_base::timed_base()
  : time_()
{}

void
timed_base::commit()
{
  time_ += time_type(do_commit(timed_duration::zero_time));
}

void
timed_base::commit(time_type const& until)
{
  if (until > time_) {
    commit(duration_type(until - time_));
  } else {
    // TODO: add warning?
    commit();
  }
}

void
timed_base::commit(duration_type const& duration)
{
  time_ += time_type(do_commit(duration));
}

timed_base::duration_type
timed_base::do_commit(duration_type duration)
{
  return duration;
}


/* ---------------------------- named_object -------------------------- */
#ifdef SYSX_NO_SYSTEMC

named_object::named_object(const char* name)
  : name_(name)
{}

const char*
named_object::name() const
{
  return name_.c_str();
}

const char*
named_object::kind() const
{
  return "object";
}

#endif // SYSX_NO_SYSTEMC

} // namespace tracing

/* Taf!
 */
