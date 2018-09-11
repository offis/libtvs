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

#include "tvs/tracing/processors/vcd_event_converter.h"

#include "tvs/tracing/timed_reader_base.h"
#include "tvs/tracing/timed_stream_base.h"
#include "tvs/tracing/timed_writer_base.h"

#include "tvs/utils/assert.h"

#include "tvs/units/time.h"

#include <boost/format.hpp>

#include <map>
#include <type_traits>

namespace tracing {

struct vcd_stream_container_base
{
  using reader_base_type = timed_reader_base;

  explicit vcd_stream_container_base(char id,
                                     std::string const& scope,
                                     std::string const& name)
    : id_(id)
    , scope_(scope)
    , name_(name)
  {}

  virtual void header_defn(std::ostream&) const = 0;
  virtual void print_front_value(std::ostream&) = 0;
  virtual void default_value(std::ostream& out) const = 0;

  virtual reader_base_type& reader() const = 0;

  char const* scope() const;

  std::string const& override_name() const { return name_; }

  virtual ~vcd_stream_container_base() = default;

protected:
  char id_;
  std::string scope_;
  std::string name_;
};

template<typename StreamType>
struct vcd_stream_container : vcd_stream_container_base
{
  using reader_type = typename StreamType::reader_type;
  using value_type = typename StreamType::value_type;

  using traits_type = vcd_traits<value_type>;

  using base_type = vcd_stream_container_base;

  vcd_stream_container(reader_type& reader,
                       std::string const& scope,
                       std::string const& name,
                       char id)
    : base_type(id, scope, name)
    , reader_(reader)
  {}

private:
  void header_defn(std::ostream& out) const override
  {

    auto nm = override_name();

    if (nm == "") {
      nm = this->reader_.stream().name();
    }

    // clang-format off
    out << "$var "
        << traits_type::header_identifier_value << " "
        << traits_type::bitwidth_value << " "
        << this->id_ << " "
        << nm << " $end\n";
    // clang-format on
  }

  reader_base_type& reader() const override { return reader_; }

  void default_value(std::ostream& out) const override
  {
    do_print_val(out, value_type());
  }

  void print_front_value(std::ostream& out) override
  {
    value_type val = this->reader_.front().value();
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
struct timed_stream_vcd_processor
  : timed_stream_processor_base
  , named_object
{
  using this_type = timed_stream_vcd_processor;
  using base_type = timed_stream_processor_base;

  using reader_type = tracing::timed_reader_base;

  using vcd_stream_ptr_type = std::unique_ptr<vcd_stream_container_base>;

public:
  timed_stream_vcd_processor(char const* modscope,
                             std::ostream& out,
                             char vcd_start_signal = 33);

  ~timed_stream_vcd_processor() override;

  template<typename T, typename Traits>
  void add(timed_stream<T, Traits>& stream, std::string scope = "")
  {
    // Decide by SFINAE if we need a converter
    this->do_add_stream(stream, scope);
  }

private:
  template<typename T>
  void do_add_stream(event_stream_type<T>& stream,
                     std::string scope,
                     std::string override_name = "")
  {
    auto conv = impl::create_converter(stream);
    auto& converted_stream = conv->stream();
    converters_.emplace_back(std::move(conv));

    // override the name to avoid the converter name in the VCD
    if (override_name == "")
      override_name = stream.name();

    this->do_add_stream(converted_stream, scope, override_name);
  }

  template<typename T, typename Traits>
  void do_add_stream(timed_stream<T, Traits>& stream,
                     std::string scope,
                     std::string override_name = "")
  {
    using stream_type = timed_stream<T, Traits>;
    using reader_type = timed_reader<T, Traits>;
    using container_type = vcd_stream_container<stream_type>;

    auto reader = std::make_unique<reader_type>(
      host::gen_unique_name("vcd_reader"), stream);

    vcd_streams_.emplace_back(std::make_unique<container_type>(
      *reader, scope, override_name, vcd_id_++));

    // move the reader (including ownership) to the backend of the processor,
    // making it sensitive to the committed values
    this->do_add_input(std::move(reader));
  }

  duration_type process(duration_type dur) override;

  void write_header();

  std::vector<vcd_stream_ptr_type> vcd_streams_;

  std::ostream& out_;
  char vcd_id_;
  bool header_written_;

  // use boost
  sysx::units::time_type scale_;

  std::vector<std::unique_ptr<impl::vcd_event_converter_base>> converters_;
};

} // namespace tracing

#endif /* TVS_TIMED_STREAM_VCD_PROCESSOR_H */
