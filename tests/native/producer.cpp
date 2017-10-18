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

#include "tvs/tracing.h"

#include "producer.h"

#include <chrono>
#include <iostream>
#include <thread>

/// provide a way to print out the enum value
std::ostream&
operator<<(std::ostream& out, producer::events const& e)
{
// clang-format off
#define _MAP(event) {producer::events::event,  #event}
  // clang-format on

  static const std::map<producer::events, const char*> cached = {
    _MAP(idle), _MAP(event_one), _MAP(event_two),
  };

#undef _MAP

  return out << cached.at(e);
}

producer::producer()
  : base_type("producer")
  , writer_("my_state_writer", tracing::STREAM_CREATE)
  , start_(clock_type::now())
{
}

void
producer::run()
{
  std::thread t1(&producer::loop, this);
  t1.join();
}

producer::events
producer::wait_for_event()
{
  auto evt = events::event_two;

  if ((rand() % 100) < 50) {
    evt = events::event_one;
  }

  auto duration = std::chrono::milliseconds(rand() % 100);
  std::this_thread::sleep_for(duration);

  return evt;
}

tracing::duration_type
producer::elapsed()
{
  using namespace std::chrono;

  // get the current time elapsed since instantiating this obj.  Ideally we
  // would use the time elapsed since the epoch (1. jan 1970), but then the
  // uint64 overflows.
  auto now = duration_cast<nanoseconds>(clock_type::now() - start_);

  sc_dt::uint64 dur = now.count() * 1e3;
  auto sc_dur = sc_core::sc_time::from_value(dur);

  return sc_dur - this->local_time();
}

void
producer::loop()
{

  while (true) {

    // block until next event arrives
    auto evt = wait_for_event();

    // How much time has elapsed since our last activity?
    auto dur = elapsed();

    writer_.push(events::idle, dur);
    writer_.push(evt, tracing::timed_duration::zero_time);
    writer_.commit(dur);

    // advance this processor's local time according to the wallclock time
    this->commit(dur);
  }
}
