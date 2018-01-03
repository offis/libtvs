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

#include "epoch.h"

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

producer::producer(char const* name)
  : base_type(name)
  , writer_("producer_writer", tracing::STREAM_CREATE)
  , start_(get_epoch())
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

  auto duration = std::chrono::microseconds(rand() % 50000);
  std::this_thread::sleep_for(duration);

  return evt;
}

sysx::units::time_type
producer::tstamp()
{
  using namespace std::chrono;

  // use nanoseconds since this day 0:00 UTC

  // get the current time elapsed since instantiating this obj.  Ideally we
  // would use the time elapsed since some absolute point in time (e.g. unix
  // epoch start) , but then the uint64 overflows since we are using a higher
  // resolution.
  auto now = duration_cast<nanoseconds>(clock_type::now() - start_);

  auto dur = (1.0 * now.count()) * sysx::si::nanoseconds;

  return dur;
}

void
producer::loop()
{
  tracing::time_type now;

  auto evt_wr = tracing::timed_var(writer_.writer());
  int loops = 10;

  while (loops-- > 0) {

    // block until next event arrives
    auto evt = wait_for_event();
    std::set<events> new_evts{ evt };

    // How long did we sleep?
    tracing::timed_duration dur = tstamp() - tracing::timed_duration(now);

    TVS_TIMED_BLOCK(dur) { evt_wr = new_evts; }

    writer_.commit(dur);
    now += tracing::time_type(dur);
  }
}
