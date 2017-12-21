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
 * \brief  Stream processor sink for recording timed-value streams to a VCD
 * file.
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>,
 *         Daniel Lorenz <daniel.lorenz@offis.de>
 *
 * This file contains the implementation for a stream processor that writes
 * timed-value tuples to a VCD file.
 */

#ifndef TVS_TIMED_STREAM_PROCESSOR_BINOP_H
#define TVS_TIMED_STREAM_PROCESSOR_BINOP_H

#include <boost/format.hpp>

#include "tvs/tracing/processors/timed_stream_processor_base.h"

#include "tvs/tracing/timed_reader_base.h"
#include "tvs/tracing/timed_stream_base.h"
#include "tvs/tracing/timed_writer_base.h"

#include "tvs/utils/assert.h"

#include "tvs/units/time.h"

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>

#include <map>

namespace tracing {

/**
 * \brief Stream processor for applying a binary operation on all stream inputs
 * and committing the result to the output sink.
 *
 * \tparam T The type of the timed_value
 * \tparam Traits The traits type of the stream
 * \tparam BinaryOperation the operation, \see std::plus for an example
 *
 */
template<typename T, typename Traits, typename BinaryOperation>
struct timed_stream_binop_processor : timed_stream_processor_base
{
  using this_type = timed_stream_binop_processor<T, Traits, BinaryOperation>;
  using base_type = timed_stream_processor_base;

  using reader_type = tracing::timed_reader<T, Traits>;
  using writer_type = tracing::timed_writer<T, Traits>;
  using stream_type = tracing::timed_stream<T, Traits>;

  using binop_type = BinaryOperation;
  using output_type = typename binop_type::result_type;

  timed_stream_binop_processor(char const* name)
    : base_type(name)
  {}

  using base_type::in;
  using base_type::out;

  void in(char const* stream)
  {
    auto str = dynamic_cast<stream_type*>(host::lookup(stream));
    SYSX_ASSERT(str != nullptr);
    this->in(*str);
  }

  void out(char const* stream)
  {
    auto str = dynamic_cast<stream_type*>(host::lookup(stream));
    SYSX_ASSERT(str != nullptr);
    this->out(*str);
  }

protected:
  /// Performs the binary operation \a BinaryOperation on all input streams for
  /// the given duration, then pushes the accumulated result to the output
  /// stream.
  duration_type process(duration_type dur) override
  {
    using namespace boost::adaptors;

    output_type result{};

    result = boost::accumulate(this->inputs() | transformed([&dur](auto& rd) {
                                 auto& reader = static_cast<reader_type&>(*rd);
                                 return reader.front(dur).value();
                               }),
                               result,
                               binop_type());

    auto& wr = static_cast<writer_type&>(this->output());

    wr.push(result, dur);

    for (auto&& i : this->inputs()) {
      i->pop();
    }

    return dur;
  }
};

#define _DECLARE_PROC(op)                                                      \
  template<typename T, typename Traits>                                        \
  using timed_stream_processor_##op =                                          \
    timed_stream_binop_processor<T, Traits, std::op<T>>

_DECLARE_PROC(plus);
_DECLARE_PROC(minus);
_DECLARE_PROC(multiplies);
_DECLARE_PROC(divides);

#undef _DECLARE_PROC

} // namespace tracing

#endif /* TVS_TIMED_STREAM_PROCESSOR_BINOP_H */
