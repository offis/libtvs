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
 * \brief  generic timed-value stream print processor class implementation
 */

#ifndef TVS_TIMED_STREAM_PRINT_PROCESSOR_H
#define TVS_TIMED_STREAM_PRINT_PROCESSOR_H

#include <tvs/tracing/processors/policies.h>
#include <tvs/tracing/processors/timed_stream_processor_base.h>

namespace tracing {

/**
 * \brief Simple stream sink for printing values to an output stream.
 *
 */
template<typename T>
using timed_stream_print_processor =
  timed_stream_processor<T, detail::print_policy>;

} // namespace tracing

#endif /* TVS_TIMED_STREAM_PRINT_PROCESSOR_H */
