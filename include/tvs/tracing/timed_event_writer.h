/*
 * Copyright (c) 2017-2018 OFFIS Institute for Information Technology
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
 * \file   timed_event_writer.h
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>
 * \brief  timed value streams for tracing (event writer interface)
 * \see    timed_value.h, timed_stream.h
 */

#ifndef TVS_TIMED_EVENT_WRITER_H
#define TVS_TIMED_EVENT_WRITER_H

#include <tvs/tracing/timed_writer.h>

#include <tvs/tracing/timed_stream_traits.h>

#include <set>

namespace tracing {

template<typename T>
using event_set_type = std::set<T, std::less<T>, std::allocator<T>>;

template<typename T>
using event_stream_type =
  tracing::timed_stream<event_set_type<T>,
                        tracing::timed_event_traits<event_set_type<T>>>;

/// Timed-Value Stream writer interface for events implemented as std::set<T>.
template<typename T>
class timed_event_writer : public timed_base
{

public:
  using value_type = std::set<T>;

  using stream_type = event_stream_type<T>;
  using writer_type = typename stream_type::writer_type;
  using tuple_type = typename stream_type::tuple_type;

  explicit timed_event_writer(stream_type& stream)
    : writer_(stream)
  {}

  explicit timed_event_writer(const char* nm, writer_mode mode = STREAM_DEFAULT)
    : writer_(nm, mode)
  {}

  /// push an event with an offset relative to the writer's local time
  void push(T const& v, duration_type const& offset)
  {
    writer_.push(timed_duration::zero_time, tuple_type({ v }, offset));

    // keep track of the maximum available duration for implcit commits
    available_dur_ = std::max(available_dur_, offset);
  }

  /// push an event at a specific time stamp
  void push(T const& v, time_type const& stamp)
  {
    SYSX_ASSERT(stamp >= local_time() &&
                "Tried to push an event into the past");

    duration_type dur = stamp - local_time();
    this->push(v, dur);
  }

  const char* name() { return writer_.name(); }

  auto& writer() { return writer_; }

  auto& stream() { return static_cast<stream_type&>(writer_.stream()); }

private:
  duration_type do_commit(duration_type until) override
  {
    // use maximum available duration in case of an implicit commit
    if (until == duration_type::zero_time) {
      until = available_dur_;
    }

    writer_.commit(until);
    available_dur_ -= until;
    return until;
  }

  duration_type available_dur_;
  writer_type writer_;
};

} // namespace tracing

namespace std {

template<typename T>
ostream&
operator<<(ostream& out, set<T> const& rhs)
{
  auto iter = rhs.begin();
  const auto end = rhs.end();

  out << "{ ";
  if (iter == end) {
    return out << "- }";
  }

  while (iter != end) {
    if (iter != rhs.begin()) {
      out << ", ";
    }
    out << *iter++;
  }
  return out << " }";
}

} // namespace std

#endif /* TVS_TIMED_EVENT_WRITER_H */
