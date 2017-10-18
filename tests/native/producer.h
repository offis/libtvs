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

#ifndef PRODUCER_H
#define PRODUCER_H

#include "tvs/tracing.h"

#include <chrono>
#include <iostream>
#include <thread>

struct producer : public tracing::timed_object
{

  using base_type = tracing::timed_object;

  using clock_type = std::chrono::high_resolution_clock;

  enum class events
  {
    idle,
    event_one,
    event_two,
  };

  producer();

  void run();

  tracing::timed_writer<events> writer_;

private:
  events wait_for_event();

  tracing::duration_type elapsed();

  void loop();

  /// the starting time of this processor
  std::chrono::time_point<clock_type> start_;
};

std::ostream& operator<<(std::ostream& out, producer::events const& e);

namespace sysx {
namespace utils {

// Provide variant support for our custom type
template <>
struct variant_traits<producer::events>
{
  using type = producer::events;

  static bool pack(variant::reference dst, type const& src)
  {
    dst.set_int(static_cast<int>(src));
    return true;
  }
  static bool unpack(type& dst, variant::const_reference src)
  {
    dst = static_cast<type>(src.get_int());
    return true;
  }
};

} // namespace utils
} // namespace sysx

#endif /* PRODUCER_H */
