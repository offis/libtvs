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

#ifndef TVS_TIMED_STREAM_VCD_EVENT_CONVERTER_H
#define TVS_TIMED_STREAM_VCD_EVENT_CONVERTER_H

#include <tvs/tracing/processors/timed_stream_vcd_processor.h>

#include <tvs/tracing/timed_reader_base.h>
#include <tvs/tracing/timed_stream_base.h>
#include <tvs/tracing/timed_writer_base.h>
#include <tvs/tracing/timed_event_writer.h>

#include <tvs/tracing/timed_stream_traits.h>

namespace tracing {

namespace impl {

class vcd_event_converter_base
{
public:
  virtual ~vcd_event_converter_base() = default;
};

// convert between std::set<T> to T streams for vcd sink
template<typename T>
class vcd_event_converter
  : public tracing::timed_listener_if
  , public vcd_event_converter_base
{
  using input_stream_type = event_stream_type<T>;
  using output_stream_type =
    tracing::timed_stream<T, tracing::timed_state_traits<T>>;

  using reader_type = typename input_stream_type::reader_type;
  using writer_type = typename output_stream_type::writer_type;

public:
  explicit vcd_event_converter(input_stream_type& input)
    : input_(tracing::host::gen_unique_name("converter_input"), input)
    , output_(host::gen_unique_name("converter_output"), tracing::STREAM_CREATE)
  {
    input_.listen(*this);
  }

  output_stream_type const& stream() const
  {
    return static_cast<output_stream_type const&>(output_.stream());
  }

  output_stream_type& stream()
  {
    return static_cast<output_stream_type&>(output_.stream());
  }

private:
  void notify(tracing::timed_reader_base&) final
  {
    while (input_.available()) {
      typename writer_type::tuple_type tup;
      auto elems = input_.get();
      if (!input_.empty()) {
        // take first element
        tup.value(*elems.begin());
      }
      tup.duration(input_.front_duration());

      // convert from event semantics to state semantics
      output_.commit(tup.duration());
      output_.push(tup.value());

      input_.pop();
    }
  }

  reader_type input_;
  writer_type output_;
};

template<typename T>
std::unique_ptr<vcd_event_converter<T>>
create_converter(event_stream_type<T>& input)
{
  return std::make_unique<vcd_event_converter<T>>(input);
}

} // namespace impl
} // namespace tracing

#endif /* TVS_TIMED_STREAM_VCD_EVENT_CONVERTER_H */
