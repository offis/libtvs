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

#include "timed_stream_fixture.h"

#include "print_processor.h"

#include "tvs/tracing.h"

#include "gtest/gtest.h"

class StreamEventSemantics : public timed_stream_fixture_b
{
  using base_type = timed_stream_fixture_b;

  using value_type = std::set<int>;
  using traits_type = tracing::timed_event_traits<value_type>;

  using stream_type = tracing::timed_stream<value_type, traits_type>;
  using printer_type = test_event_printer<int>;

  // special event writer to simplify event handling with std::set<T>
  using writer_type = tracing::timed_event_writer<int>;

protected:
  StreamEventSemantics()
    : base_type()
    , writer("writer", tracing::STREAM_CREATE)
  {
    printer.in(writer.name());
  }

  void expect_processor_output(std::string const& str)
  {
    std::stringstream actual;
    printer.print(actual);
    EXPECT_EQ(str, actual.str());
    printer.clear();
  }

  writer_type writer;
  printer_type printer;
};

TEST_F(StreamEventSemantics, PushRelEvent)
{
  // Push two events at the relative offset dur
  writer.push(0, dur);
  writer.push(10, dur);
  writer.commit();

  expect_processor_output("@1 s: { 0, 10 }\n");
}

TEST_F(StreamEventSemantics, PushAbsEvent)
{
  // Push two events to an absolute time point
  tracing::time_type abs{ dur };
  writer.push(0, abs);
  writer.push(10, abs);
  writer.commit();
  expect_processor_output("@1 s: { 0, 10 }\n");
}

TEST_F(StreamEventSemantics, PushSemantics)
{
  // Push two events at the relative offset dur
  writer.push(0, dur);
  writer.push(10, 2 * dur);
  writer.commit();
  expect_processor_output("@1 s: { 0 }\n"
                          "@2 s: { 10 }\n");
}

TEST_F(StreamEventSemantics, PartialCommit)
{
  // Push two events to an absolute time point which need to be 'merged'
  tracing::time_type abs{ dur };
  writer.push(0, abs);
  writer.push(10, abs / 2.0);

  // note that we use the absolute time commit interface here
  writer.commit(abs / 2.0);
  writer.commit(abs);

  // a split/merge results in decaying of the old event value to the 'right'
  // side while the duration and an empty value is created on the 'left' side of
  // a split
  std::stringstream exp;
  exp << "@" << dur / 2.0 << ": { 10 }\n";
  exp << "@" << dur << ": { 0 }\n";

  expect_processor_output(exp.str());
}

TEST_F(StreamEventSemantics, SplitMergeSemantics)
{
  // Push two events to an absolute time point which need to be 'merged'
  tracing::time_type abs{ dur };
  writer.push(0, abs);
  writer.push(10, abs / 2.0);
  writer.commit();

  // a split/merge results in decaying of the old event value to the 'right'
  // side while the duration and an empty value is created on the 'left' side of
  // a split

  std::stringstream exp;
  exp << "@" << dur / 2.0 << ": { 10 }\n";
  exp << "@" << dur << ": { 0 }\n";

  expect_processor_output(exp.str());
}

TEST_F(StreamEventSemantics, SplitMergeSemanticsTwo)
{
  // Push two events to an absolute time point which need to be 'merged'
  tracing::time_type abs{ dur };
  writer.push(0, abs);
  writer.push(5, abs);
  writer.push(10, abs / 2.0);
  writer.push(10, abs / 4.0);
  writer.commit();

  // we expect to have the events ordered correctly
  std::stringstream exp;
  exp << "@" << dur / 4.0 << ": { 10 }\n";
  exp << "@" << dur / 2.0 << ": { 10 }\n";
  exp << "@" << dur << ": { 0, 5 }\n";

  expect_processor_output(exp.str());
}

TEST_F(StreamEventSemantics, SplitMergeSemanticsThree)
{
  tracing::time_type abs{ dur };
  writer.push(0, abs);
  writer.push(5, abs);
  writer.push(10, abs / 2.0);
  writer.push(10, abs / 4.0);

  // now, push again over the whole 'duration'
  writer.push(10, abs);
  writer.commit();

  std::stringstream exp;
  exp << "@" << dur / 4.0 << ": { 10 }\n";
  exp << "@" << dur / 2.0 << ": { 10 }\n";
  exp << "@" << dur << ": { 0, 5, 10 }\n";

  expect_processor_output(exp.str());
}

TEST_F(StreamEventSemantics, SplitMergeCommitSemantics)
{
  // Push two events to an absolute time point which need to be 'merged'
  tracing::time_type abs{ dur };
  writer.push(0, abs);
  writer.push(5, abs);
  writer.push(10, abs / 2.0);
  writer.push(10, abs / 4.0);

  // now further split the stream by performing partial commits
  writer.commit(dur / 8.0);
  writer.commit(dur / 8.0);

  writer.commit(dur / 4.0);
  writer.commit(dur / 4.0);

  // the last commit should update up to the last event of the pushed events
  writer.commit();

  // we expect to have the events ordered correctly and have the splits at the
  // commit boundaries
  std::stringstream exp;
  exp << "@" << dur / 8.0 << ": { - }\n";
  exp << "@" << dur / 4.0 << ": { 10 }\n";
  exp << "@" << dur / 2.0 << ": { 10 }\n";
  exp << "@" << dur * (3.0 / 4.0)  << ": { - }\n";
  exp << "@" << dur << ": { 0, 5 }\n";

  expect_processor_output(exp.str());
}

tracing::time_type operator"" _ns(unsigned long long val)
{
#ifdef SYSX_NO_SYSTEMC
  return tracing::time_type(val * 1.0 * sysx::si::nanoseconds );
#else
  return tracing::time_type(1.0 * val, sc_core::SC_NS);
#endif
}


tracing::time_type operator"" _ms(unsigned long long val)
{
#ifdef SYSX_NO_SYSTEMC
  return tracing::time_type(val * 1.0 * sysx::si::milliseconds );
#else
  return tracing::time_type(1.0 * val, sc_core::SC_MS);
#endif
}

TEST_F(StreamEventSemantics, RoundingErrorTests)
{
  writer.push(0, 251_ns);
  writer.push(0, 46_ns);
  writer.push(0, 209_ns);
  writer.commit();
  EXPECT_EQ(writer.local_time(), 251_ns);
}

TEST_F(StreamEventSemantics, RoundingErrorTests2)
{
  writer.push(0, 52_ns);
  writer.push(0, 126_ns);
  writer.push(0, 248_ns);
  writer.push(0, 22_ns);
  writer.push(0, 26_ns);
  writer.push(0, 253_ns);

  writer.commit();
  EXPECT_EQ(writer.local_time(), 253_ns);
}
