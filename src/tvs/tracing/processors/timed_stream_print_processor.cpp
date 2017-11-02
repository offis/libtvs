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
 * \brief  generic timed-value stream print processor class implementation
 */

#include "tvs/tracing/processors/timed_stream_print_processor.h"

#include "tvs/tracing/timed_duration.h"

namespace tracing {

timed_stream_print_processor::timed_stream_print_processor(char const* name,
                                                           std::ostream& out)
  : base_type(name)
  , output_(out)
{}

timed_stream_print_processor::~timed_stream_print_processor() {}

timed_stream_print_processor::duration_type
timed_stream_print_processor::process(duration_type dur)
{
  for (auto&& reader : this->inputs()) {
    output_ << reader->local_time() << ":";
    print_tuple(output_, reader->front(dur));
    output_ << std::endl;
    reader->pop();
  }
  return dur;
}

} // namespace tracing
