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
timed_stream_processor_base::notify(reader_base_type&)
{
  const auto availp = [](auto const& r) { return r->available(); };

  const auto time_cmp = [](auto const& lhs, auto const& rhs) {
    return lhs->available_until() < rhs->available_until();
  };

  const auto begin = inputs_.cbegin();
  const auto end = inputs_.cend();

  while (std::all_of(begin, end, availp)) {

    // determine minimum front value duration on each run
    auto const& stream = std::min_element(begin, end, time_cmp);
    auto duration = (*stream)->front_duration();

    // let the user process all readers with the minimum front duration
    auto advance = process(duration);

    // wait until next token arrives when no duration was returned
    if (advance == duration_type::zero_time)
      break;

    // update this stream processor's local time after processing
    commit(advance);
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
