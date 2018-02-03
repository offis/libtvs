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

#ifndef TVS_TIMED_STREAM_VCD_PROCESSOR_H
#define TVS_TIMED_STREAM_VCD_PROCESSOR_H

#include "tvs/tracing/processors/timed_stream_processor_base.h"

#include "tvs/tracing/timed_reader_base.h"
#include "tvs/tracing/timed_stream_base.h"
#include "tvs/tracing/timed_writer_base.h"

#include "tvs/utils/assert.h"

#include "tvs/units/time.h"

#include <boost/format.hpp>

#include <map>

namespace tracing {

/**
 * \brief Simple stream sink for printing values to an output stream.
 *
 * \tparam T The type of the timed_value
 * \tparam Traits The traits type of the stream
 */
struct timed_stream_vcd_processor : timed_stream_processor_base, named_object
{
  using this_type = timed_stream_vcd_processor;
  using base_type = timed_stream_processor_base;

  using reader_type = tracing::timed_reader_base;
  using duration_type = typename base_type::duration_type;

public:
  timed_stream_vcd_processor(char const* modscope,
                             std::ostream& out,
                             char vcd_start_signal = 'a');

  ~timed_stream_vcd_processor();

  /// FIXME: refactor with generic base interface
  template<typename T, typename Policy>
  void add(std::string name,
           tracing::timed_stream<T, Policy> const& stream,
           std::string type,
           unsigned int bitwidth)
  {
    this->add<T, Policy>(name, stream.name(), type, bitwidth);
  }

  /// FIXME: refactor with generic base interface
  template<typename T, typename Policy>
  void add(const char* name,
           const char* stream_name,
           std::string type,
           unsigned int bitwidth)
  {
    using reader_type = tracing::timed_reader<T, Policy>;

    std::stringstream vcd_reader_name;
    vcd_reader_name << "reader_" << name;

    auto reader = detail::make_unique<reader_type>(
      vcd_reader_name.str().c_str(), stream_name);

    this->add_vcd(reader.get(), name, type, bitwidth);
    this->do_add_input(std::move(reader));
  }

protected:
  duration_type process(duration_type dur) override;

private:
  void add_vcd(reader_base_type* reader,
               const char* name,
               std::string type,
               unsigned int bitwidth);

  void write_header();

  struct timed_stream_vcd_impl;
  const std::unique_ptr<timed_stream_vcd_impl> pimpl_;

  std::ostream& out_;
  bool header_written_;
};

} // namespace tracing

#endif /* TVS_TIMED_STREAM_VCD_PROCESSOR_H */
