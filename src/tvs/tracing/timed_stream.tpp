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
 * \file   timed_stream.tpp
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed value stream (template implementation)
 * \see    timed_stream.h
 */

#include "tvs/tracing/timed_reader.h"
#include "tvs/tracing/timed_stream.h"
#include "tvs/tracing/timed_writer.h"

#include "tvs/tracing/timed_stream_traits.h"

#include "tvs/utils/unique_ptr.h"

#include "tvs/utils/debug.h"
#include "tvs/utils/macros.h"

namespace tracing {

/* -------------------------- push interface -------------------------- */

template<typename T, template<typename> class P>
void
timed_stream<T, P>::push(tuple_type const& t)
{
  if (future_.empty()) {
    buf_.push_back(t);
  } else {

    // consume any future values caused by the local offset increment
    sequence_type tmp;
    tmp.push_back(t);
    future_.merge(tmp);

    auto merge_range = future_.range(t.duration());

    SYSX_ASSERT(merge_range.duration() == t.duration());

    buf_.push_back(merge_range.begin(), merge_range.end());
    future_.pop_front(merge_range.duration());
  }
}

template<typename T, template<typename> class P>
void
timed_stream<T, P>::push(value_type const& val)
{
  tuple_type tup(val, duration_type::infinity());

  if (future_.front().is_infinite()) {
    future_.front(tup);
  } else {
    // merge with existing future sequence
    sequence_type pushed;
    pushed.push_back(tup);
    future_.merge(pushed);
  }
}

template<typename T, template<typename> class P>
void
timed_stream<T, P>::push(time_type offset, tuple_type const& tuple)
{
  sequence_type pushed;

  if (offset > duration_type::zero_time)
    pushed.push_back(empty_policy::empty(offset));

  pushed.push_back(tuple);
  future_.merge(pushed);
}

/* ------------------------- commit interface ------------------------- */

template<typename T, template<typename> class P>
void
timed_stream<T, P>::do_pre_commit_reader(duration_type const& dur)
{
  // make partial commit possible
  if (dur <= buf_.duration()) {
    buf_.split(dur);
    return;
  }

  // add future values to satisfy the requested duration

  duration_type fdur = dur - buf_.duration();
  future_.split(fdur);

  // append from future so we can satisfy the commit
  auto range = future_.range(fdur);

  sequence_type tmp;
  tmp.push_back(range.begin(), range.end());
  buf_.push_back(tmp);

  future_.pop_front(fdur);
}

template<typename T, template<typename> class P>
void
timed_stream<T, P>::do_commit_reader(timed_reader_base& r,
                                     duration_type const& dur,
                                     bool last)
{
  typedef timed_reader<T, P> reader_type;
  reader_type& reader = static_cast<reader_type&>(r);

  if (dur == duration()) {
    if (!last) {
      reader.buf_.push_back(buf_);
    } else {
      reader.buf_.move_back(buf_);
    }
  } else {
    // partially commit, buf_ has already been prepared
    auto range = buf_.range(dur);
    reader.buf_.push_back(range.begin(), range.end());

    if (last)
      buf_.pop_front(dur);
  }

  bool new_window = reader.buf_.empty();
  reader.trigger(new_window);
}

} // namespace tracing

/* Taf!
 */
