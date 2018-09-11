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
 * \brief  Policies for handling incoming timed_value tokens in a processor.
 */

#ifndef TVS_PROCESSOR_POLICIES_H
#define TVS_PROCESSOR_POLICIES_H

#include <tvs/tracing/processors/timed_stream_processor_base.h>

#include <tvs/tracing/timed_duration.h>

#include <set>

namespace tracing {

class timed_value_base;
class timed_reader_base;
class timed_duration;

namespace detail {

template<typename T>
struct process_event_policy
{
  using value_type = T;
  using set_type = std::set<T>;

  virtual void do_process_events(set_type const& val,
                                 time_type const& stamp) = 0;

  void process(timed_reader_base& in, timed_duration dur)
  {
    time_type stamp( in.local_time() + dur );
    auto val = static_cast<const timed_value<set_type>&>(in.front(dur));
    this->do_process_events(val.value(), stamp);
  }

protected:
  ~process_event_policy() = default;
};

template<typename T>
struct print_policy
{
  using tuple_type = timed_value<T>;

protected:
  ~print_policy() = default;

  void set_ostream(std::ostream& out) { output_ = &out; }

  void process(timed_reader_base& in, timed_duration const& dur)
  {
    SYSX_ASSERT(output_ != nullptr);
    while (in.available()) {
      auto val = static_cast<tuple_type const&>(in.front());
      this->do_print_tuple(*output_, val, in.local_time());
      in.pop();
    }
  }

private:
  /// override this to adjust tuple print behaviour
  virtual void do_print_tuple(std::ostream& out,
                              tuple_type const& val,
                              time_type const& local_time)
  {
    out << local_time << ":" << val << "\n";
  }

  std::ostream* output_{ nullptr };
};

} // namespace detail
} // namespace tracing

#endif /* TVS_PROCESSOR_POLICIES_H */
