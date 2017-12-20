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


#ifndef POWER_PRODUCER_H
#define POWER_PRODUCER_H

#include "tvs/tracing.h"

#include <systemc>

#include <cstdlib>
#include <map>

// Component that produces a power stream depending on a state machine with
// three states and random transitions.
class power_producer : public sc_core::sc_module
{
  enum class power_state
  {
    idle,
    low_power,
    high_performance
  };

  tracing::timed_writer<double, tracing::timed_process_traits<double>> writer_;
  const std::map<power_state, double> power_values_;

public:
  SC_HAS_PROCESS(power_producer);
  power_producer(sc_core::sc_module_name nm, double p1, double p2, double p3)
    : sc_module(nm)
    , writer_("my_power_writer", tracing::STREAM_CREATE)
    , power_values_{
      { power_state::idle, p1 },
      { power_state::low_power, p2 },
      { power_state::high_performance, p3 },
    }
  {
    SC_THREAD(produce);
  }

  void produce()
  {
    auto power_value = tracing::timed_var(writer_);
    while (true) {

#ifdef SYSX_NO_SYSTEMC
      tracing::time_type duration(rand() % 100 * 1.0 * sysx::si::nanoseconds);
#else
      tracing::time_type duration(rand() % 100, sc_core::SC_NS);
#endif

      // block-based annotation
      TVS_TIMED_BLOCK(duration)
      {
        int temp = rand() % 30;
        if (temp < 10)
          power_value = power_values_.at(power_state::idle);
        else if (temp < 20)
          power_value = power_values_.at(power_state::low_power);
        else
          power_value = power_values_.at(power_state::high_performance);
      }

      // synchronise SystemC and stream time domains
      TVS_SYNCHRONISATON_POINT();
    }
  }

}; // class power_producer

#endif // POWER_PRODUCER_H
/* Taf!
 * :tag: (streamprocessingtest,h)
 */
