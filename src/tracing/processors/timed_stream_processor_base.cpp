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

timed_stream_processor_base::timed_stream_processor_base() = default;

void
timed_stream_processor_base::update_cache()
{

  // invalidate cache
  available_inputs_.clear();
  front_duration_ = duration_type::infinity();

  for (auto const& it : inputs()) {
    auto const* ptr = &(*it);
    if (!it->available()) {
      break;
    }

    available_inputs_.insert(ptr);

    // re-set minimum duration of all incoming available readers
    front_duration_ = std::min(front_duration_, it->front_duration());
  }
}

void
timed_stream_processor_base::notify(reader_base_type& rd)
{
  // check if all readers have notified
  available_inputs_.insert(&rd);

  // remember the minimum duration of an incoming reader (since it also must
  // have tokens available)
  front_duration_ = std::min(front_duration_, rd.front_duration());

  // run as long as there are readers with available tokens
  while (available_inputs_.size() == inputs_.size()) {
    // consume until no more duration is available or until the process() stops
    // advancing
    duration_type consumed;
    do {
      auto const& advance = process(front_duration_ - consumed);
      consumed += advance;

      if (advance == duration_type::zero_time)
        break;
    } while (consumed < front_duration_);
    commit(consumed);

    // updates front_duration_ and available_inputs_
    update_cache();
  }
}

timed_stream_processor_base::duration_type
timed_stream_processor_base::do_commit(duration_type until)
{
  time_type stamp = local_time() + until;
  for (auto&& out : outputs())
    out->commit(stamp);

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
