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

#include <tvs/tracing/processors/timed_stream_processor_base.h>

#include <tvs/tracing/processors/vcd_traits.h>

#include <tvs/tracing/processors/vcd_event_converter.h>

#include <tvs/tracing/timed_reader_base.h>
#include <tvs/tracing/timed_stream_base.h>
#include <tvs/tracing/timed_writer_base.h>

#include <tvs/utils/assert.h>

#include <tvs/units/time.h>

#include <boost/format.hpp>

#include <map>
#include <type_traits>

namespace tracing {

struct vcd_stream_container_base
{
  using reader_base_type = timed_reader_base;

  using id_type = std::string;

  explicit vcd_stream_container_base(id_type const& id,
                                     std::string const& scope,
                                     std::string const& name)
    : id_(id)
    , scope_(scope)
    , name_(name)
  {}

  virtual void print_node_information(std::ostream&) const = 0;
  virtual void print_front_value(std::ostream&) const = 0;
  virtual void print_default_value(std::ostream& out) const = 0;

  /// returns whether the front value is different from the last value that has
  /// been printed
  virtual bool value_changed() const = 0;

  /// update the last value that has been printed
  virtual void update_value() = 0;

  virtual reader_base_type& reader() const = 0;

  char const* scope() const;

  std::string const& override_name() const { return name_; }

  virtual ~vcd_stream_container_base() = default;

protected:
  id_type id_;
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
                       id_type const& id)
    : base_type(id, scope, name)
    , reader_(reader)
    , prev_(value_type())
  {}

private:
  void print_node_information(std::ostream& out) const override
  {
    auto nm = override_name();
    if (nm == "") {
      nm = this->reader_.stream().name();
    }

    SYSX_ASSERT(traits_type::bitwidth_value >= 1);

    // bug(?): using the constexpr values directly in the boost::format call
    // results in a linker error
    constexpr const char* idval = traits_type::header_identifier_value;
    constexpr uint16_t bitwidth = traits_type::bitwidth_value;

    // clang-format off
    if (traits_type::bitwidth_value == 1) {
      out << boost::format("$var %s  % 3d  %s  %s         $end\n")
                           % idval
                           % bitwidth
                           % this->id_
                           % nm;
    } else {
      out << boost::format("$var %s  % 3d  %s  %s [%d:0]  $end\n")
                           % idval
                           % bitwidth
                           % this->id_
                           % nm
                           % (bitwidth - 1);
    }
    // clang-format on
  }

  reader_base_type& reader() const override { return reader_; }

  void print_default_value(std::ostream& out) const override
  {
    do_print_val(out, value_type());
  }

  void print_front_value(std::ostream& out) const override
  {
    do_print_val(out, this->reader_.get());
  }

  bool value_changed() const override { return prev_ != this->reader_.get(); }

  void update_value() override { prev_ = this->reader_.get(); }

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
  value_type prev_;
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
  timed_stream_vcd_processor(char const* modscope, std::ostream& out);

  ~timed_stream_vcd_processor() override;

  template<typename T, typename Traits>
  void add(timed_writer<T, Traits>& writer, std::string scope = "")
  {
    using stream_type = timed_stream<T, Traits>;
    // Decide by SFINAE if we need a converter
    this->add(static_cast<stream_type&>(writer.stream()), scope);
  }

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
      *reader, scope, override_name, next_identifier()));

    // move the reader (including ownership) to the backend of the processor,
    // making it sensitive to the committed values
    this->do_add_input(std::move(reader));
  }

  std::string next_identifier();

  /// prints the timestamp to the output buffer
  void print_timestamp(time_type const&);

  void notify(reader_base_type&) override;

  void write_header();

  std::ostream& out_;

  std::vector<vcd_stream_ptr_type> vcd_streams_;

  std::uint64_t vcd_id_{ 0 };
  bool header_written_{ false };

  // use boost
  sysx::units::time_type scale_{ 1.0 * sysx::si::picoseconds };

  // temporary stringstream to print the VCD values
  std::stringstream temp_sstr_{};

  std::vector<std::unique_ptr<impl::vcd_event_converter_base>> converters_;
};

} // namespace tracing

#endif /* TVS_TIMED_STREAM_VCD_PROCESSOR_H */
