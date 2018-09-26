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
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>
 * \brief  generic timed-value stream processor base class implementation
 */

#include "tvs/tracing/processors/timed_stream_processor_base.h"

#include "tvs/tracing/timed_reader_base.h"
#include "tvs/tracing/timed_stream_base.h"
#include "tvs/tracing/timed_writer_base.h"

#include "tvs/utils/systemc.h"

#include "tvs/utils/assert.h"
#include "tvs/utils/unique_ptr.h"

namespace tracing {

namespace {

// helper function to update the minimum duration cache entry
void
set_min_duration(duration_type& min_dur, duration_type const& dur)
{
  if (min_dur == duration_type::zero_time) {
    min_dur = dur;
  } else {
    min_dur = std::min(min_dur, dur);
  }
};

} // anonymous namespace

timed_stream_processor_base::timed_stream_processor_base() = default;

void
timed_stream_processor_base::notify(reader_base_type& rd)
{

  // remember minimum available duration of all incoming readers
  set_min_duration(available_duration_, rd.available_duration());

  // check if all readers have notified
  available_inputs_.insert(&rd);
  if (available_inputs_.size() != inputs_.size()) {
    return;
  }

  if (available_duration_ == duration_type::zero_time)
    return;

  // consume until no more duration is available or until the process() stops
  // advancing
  duration_type consumed;
  while (consumed < available_duration_) {
    auto const& advance = process(available_duration_ - consumed);
    if (advance == duration_type::zero_time)
      break;
    consumed += advance;
  }

  commit(consumed);

  // invalidate cache
  available_inputs_.clear();
  available_duration_ = duration_type::zero_time;

  // re-build the cache, since we don't know which input readers process() has
  // fully consumed or what the new minimum duration is.
  for (auto&& it : inputs_) {
    auto const* ptr = &(*it);
    if (it->available()) {
      available_inputs_.insert(ptr);

      // re-set minimum duration of all incoming readers
      set_min_duration(available_duration_, it->available_duration());
    }
  }
}

timed_stream_processor_base::duration_type
timed_stream_processor_base::do_commit(duration_type until)
{
  for (auto&& out : outputs())
    out->commit(until);

  return until;
}

void
timed_stream_processor_base::do_add_input(reader_ptr_type&& reader)
{
  reader->listen(*this);
  inputs_.emplace_back(std::move(reader));
}

void
timed_stream_processor_base::do_add_output(writer_ptr_type&& writer)
{
  outputs_.emplace_back(std::move(writer));
}

} // namespace tracing
