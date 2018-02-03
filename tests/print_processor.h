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

#ifndef PRINT_PROCESSOR_H_INCLUDED_
#define PRINT_PROCESSOR_H_INCLUDED_

#include "tvs/tracing.h"

#include <ostream>
#include <sstream>

/**
 * \brief Simple printer that buffers the underlying print processor output.
 *
 * \tparam T The type of the timed_value
 * \tparam Traits The traits type of the stream
 */
template<typename T>
class test_printer : public tracing::timed_stream_print_processor<T>
{
  using this_type = test_printer<T>;
  using base_type = tracing::timed_stream_print_processor<T>;

public:
  enum output_type
  {
    OUTPUT_BUFFERED = 0,
    OUTPUT_COUT,
  };

  test_printer(output_type out = OUTPUT_BUFFERED)
    : buf_()
  {
    base_type::set_ostream(out == OUTPUT_COUT ? std::cout : buf_);
  }

  using base_type::in;

  void print(std::ostream& out) const { out << buf_.str(); }

  void clear()
  {
    buf_.clear();
    buf_.str(std::string());
  }

private:
  std::stringstream buf_;
};

template<typename EventT>
class test_event_printer : public test_printer<std::set<EventT>>
{
  using value_type = std::set<EventT>;
  using this_type = test_event_printer<EventT>;
  using base_type = test_printer<value_type>;

public:
  using output_type = typename base_type::output_type;
  using tuple_type = tracing::timed_value<value_type>;

  using base_type::in;

  test_event_printer(output_type out = base_type::OUTPUT_BUFFERED)
    : base_type(out)
  {
  }

  void in(const char* name)
  {
    using stream_type =
      tracing::timed_stream<value_type,
                            tracing::timed_event_traits<value_type>>;

    base_type::in(tracing::stream_by_name<stream_type>(name));
  }

private:
  virtual void do_print_tuple(std::ostream& out,
                              tuple_type const& val,
                              tracing::time_type const& local_time) override
  {
    out << "@" << local_time + val.duration() << ": " << val.value() << "\n";
  }
};

#endif // PRINT_PROCESSOR_H_INCLUDED_
