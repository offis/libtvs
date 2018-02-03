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

#include "tvs/tracing/processors/vcd_traits.h"

#include "tvs/tracing/timed_reader_base.h"
#include "tvs/tracing/timed_stream_base.h"
#include "tvs/tracing/timed_writer_base.h"

#include "tvs/utils/assert.h"

#include "tvs/units/time.h"

#include <boost/format.hpp>

#include <map>

namespace tracing {

struct vcd_stream_container_base
{
  using reader_base_type = timed_reader_base;

  explicit vcd_stream_container_base(char id)
    : id_(id)
  {}

  virtual void header_defn(std::ostream&) const = 0;
  virtual void print_value(std::ostream&, duration_type const&) = 0;
  virtual void default_value(std::ostream& out) const = 0;

  virtual reader_base_type& reader() = 0;

  virtual char const* scope() const = 0;

  virtual ~vcd_stream_container_base() {}

protected:
  char id_;
};

template<typename StreamType>
struct vcd_stream_container : vcd_stream_container_base
{
  using reader_type = typename StreamType::reader_type;
  using value_type = typename StreamType::value_type;

  using traits_type = vcd_traits<value_type>;

  using base_type = vcd_stream_container_base;

  vcd_stream_container(reader_type& reader, char id)
    : base_type(id)
    , reader_(reader)
  {}

private:
  void header_defn(std::ostream& out) const override
  {
    // clang-format off
    out << "$var "
        << traits_type::header_identifier_value << " "
        << traits_type::bitwidth_value << " "
        << this->id_ << " "
        << this->reader_.stream().name() << " $end\n";
    // clang-format on
  }

  char const* scope() const override
  {
    return this->reader_.stream().get_parent_object()->name();
  }

  reader_base_type& reader() override { return reader_; }

  void default_value(std::ostream& out) const override
  {
    do_print_val(out, value_type());
  }

  void print_value(std::ostream& out, duration_type const& dur) override
  {
    value_type val = this->reader_.front(dur).value();
    do_print_val(out, val);
  }

  void do_print_val(std::ostream& out, value_type const& val) const
  {
    if (traits_type::bitwidth_value == 1) {
      traits_type::print(out, val);
    } else {
      out << traits_type::trace_identifier_value;
      traits_type::print(out, val);
      out << " ";
    }

    out << this->id_ << "\n";
  }

  reader_type& reader_;
};

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

  using vcd_stream_ptr_type = std::unique_ptr<vcd_stream_container_base>;

public:
  timed_stream_vcd_processor(char const* modscope,
                             std::ostream& out,
                             char vcd_start_signal = 33);

  ~timed_stream_vcd_processor();

  template<typename T, typename Traits>
  void add(timed_stream<T, Traits>& stream)
  {
    using stream_type = timed_stream<T, Traits>;
    using reader_type = timed_reader<T, Traits>;
    using container_type = vcd_stream_container<stream_type>;

    std::stringstream ss;
    ss << "vcd_reader_" << stream.basename();
    auto reader = std::make_unique<reader_type>(ss.str().c_str(), stream);

    vcd_streams_.emplace_back(
      std::make_unique<container_type>(*reader, vcd_id_++));

    // move the reader (including ownership) to the backend of the processor,
    // making it sensitive to the committed values
    this->do_add_input(std::move(reader));
  }

private:
  duration_type process(duration_type dur) override;

  void write_header();

  std::vector<vcd_stream_ptr_type> vcd_streams_;

  std::ostream& out_;
  char vcd_id_;
  bool header_written_;

  // use boost
  sysx::units::time_type scale_;
};

} // namespace tracing

#endif /* TVS_TIMED_STREAM_VCD_PROCESSOR_H */
