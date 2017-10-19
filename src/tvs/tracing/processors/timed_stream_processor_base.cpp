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

timed_stream_processor_base::timed_stream_processor_base(const char* name)
  : base_type(name)
{
}

timed_stream_processor_base::writer_base_type&
timed_stream_processor_base::output()
{
  SYSX_ASSERT(output_);
  return *output_.get();
}

void
timed_stream_processor_base::notify(reader_base_type&)
{
  const auto availp = [](reader_ptr_type const& r) { return r->available(); };

  const auto duration_cmp = [](reader_ptr_type const& lhs,
                               reader_ptr_type const& rhs) {
    return lhs->available_duration() < rhs->available_duration();
  };

  if (std::all_of(inputs_.cbegin(), inputs_.cend(), availp)) {

    // update this output stream by advancing for the minimum duration available
    // across all input streams
    auto const stream =
      std::min_element(inputs_.cbegin(), inputs_.cend(), duration_cmp);

    SYSX_ASSERT(stream != inputs_.cend());

    auto duration = (*stream)->available_duration();

    // update this stream processor's local time after processing
    commit(process(duration));
  }
}

timed_stream_processor_base::duration_type
timed_stream_processor_base::do_commit(duration_type until)
{
  if (output_)
    output_->commit(until);

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
  output_ = std::move(writer);
}

} // namespace tracing
