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

#include "power_producer.h"

#include "timed_stream_fixture.h"

#include "tvs/tracing.h"
#include "gtest/gtest.h"

#include <ostream>

#include <systemc>

SC_MODULE(testbench)
{
  power_producer p1, p2, p3;
  using traits_type = tracing::timed_process_traits<double>;

  tracing::timed_stream_vcd_processor vcd;

  tracing::timed_stream_processor_plus<double, traits_type> proc_plus;

  tracing::timed_stream<double, traits_type> add_result;

  testbench(sc_core::sc_module_name nm, std::ostream & out)
    : sc_module(nm)
    , p1("p1", 1, 2, 3)
    , p2("p2", 4, 5, 6)
    , p3("p3", 7, 8, 9)
    , vcd("sink", out)
    , proc_plus("proc_plus")
    , add_result("add_result")
  {
    vcd.add<double, traits_type>("power_from_p1", "p1.my_power_writer", "real", 31);
    vcd.add<double, traits_type>("power_from_p2", "p2.my_power_writer", "real", 31);
    vcd.add<double, traits_type>("power_from_p3", "p3.my_power_writer", "real", 31);

    proc_plus.in("p1.my_power_writer");
    proc_plus.in("p2.my_power_writer");
    proc_plus.out(add_result);
  }
};

struct StreamProcessorVCDTests
  : public timed_stream_fixture<double, testbench::traits_type>
{
protected:
  testbench tb;
  std::stringstream teststream;

  StreamProcessorVCDTests()
    : tb("tb", teststream)
  {
    printer.in("tb.add_result");
    sc_start(1, sc_core::SC_US);

    std::cout << teststream.str();
  }
};

TEST_F(StreamProcessorVCDTests, CheckHeader)
{
  std::string correct = "$timescale 1 ps $end\n"
                        "$scope module tb.sink $end\n"
                        "$var real 31 a power_from_p1 $end\n"
                        "$var real 31 b power_from_p2 $end\n"
                        "$var real 31 c power_from_p3 $end\n"
                        "$upscope $end\n"
                        "$enddefinitions $end";

  std::string header(correct.size(), ' ');
  teststream.read(&header[0], correct.size());
  EXPECT_EQ(header, correct);
}

/* Taf!
 * :tag: (streamprocessingtest,s)
 */
