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

class StreamStateSemantics
  : public timed_stream_fixture<int, tracing::timed_state_traits<int>>
{
protected:
  StreamStateSemantics()
    : test_tuple(4711, dur)
  {
  }

  const tuple_type test_tuple;
};

// joining tuples with state behaviour should result in the same values of the
// joined tuple
TEST_F(StreamStateSemantics, JoinPolicy)
{
  writer.push(test_tuple);
  writer.push(test_tuple);
  writer.commit();
  expect_processor_output("0 s:(4711,2 s)\n");
}

// merging tuples with state behaviour should result in the same values of the
// new tuples
TEST_F(StreamStateSemantics, SplitPolicy)
{
  writer.push(4711, dur * 2);
  writer.commit(dur);
  expect_processor_output("0 s:(4711,1 s)\n");
  writer.commit(dur);
  expect_processor_output("1 s:(4711,1 s)\n");
}

// merging tuples with state behaviour should fail
TEST_F(StreamStateSemantics, MergePolicyDeath)
{
  ASSERT_DEATH(
    {
      writer.push(dur, 4711, dur);
      writer.push(4711, dur * 2);
      writer.commit(dur);
    },
    "");
}

TEST_F(StreamStateSemantics, PushDuration)
{
  writer.push(test_tuple);
  writer.commit();
  expect_processor_output("0 s:(4711,1 s)\n");

  writer.push(4711, dur * 2);
  writer.commit();
  expect_processor_output("1 s:(4711,2 s)\n");
}

// -------------------------------------------------------------------

// Pushing future tuples to a stream should not advance the local time and
// therefore not commit anything if no commit duration is given.
TEST_F(StreamStateSemantics, PushOffsetAndCommitWithoutDuration)
{
  writer.push(dur, 4711, dur);
  EXPECT_EQ(zero_time, writer.local_time());

  writer.commit();
  EXPECT_EQ(zero_time, writer.end_time());
  expect_processor_output("");
}

// Test partially committing in a state-based stream
TEST_F(StreamStateSemantics, PartialCommitTests)
{
  writer.push(4711);
  writer.commit(dur * 2);
  writer.push(4712);
  writer.commit(dur);
  writer.push(4713);
  writer.commit(dur * 3);

  expect_processor_output("0 s:(4711,2 s)\n"
                          "2 s:(4712,1 s)\n"
                          "3 s:(4713,3 s)\n");
}

// same for indefinite pushes
TEST_F(StreamStateSemantics, PushIndefiniteAndCommitWithoutDuration)
{
  writer.push(4711);
  EXPECT_EQ(zero_time, writer.end_time());

  writer.commit();
  EXPECT_EQ(zero_time, writer.end_time());
  expect_processor_output("");
}

// test push zero time
TEST_F(StreamStateSemantics, DISABLED_PushAndCommitWithZeroTimeState)
{
  writer.push(4711, dur);
  writer.push(4712, zero_time);
  writer.commit();
  expect_processor_output("0 s:(4711,1 s)\n"
                          "1 s:(4712,0 s)\n");
}

// Pushing with an offset followed by a commit should fill up the empty interval
// as defined by the empty_policy
TEST_F(StreamStateSemantics, PushOffset)
{
  writer.push(dur, 4711, dur);
  writer.commit(2 * dur);
  EXPECT_EQ(2 * dur, writer.end_time());

  // local time offset should be resetted
  EXPECT_EQ(zero_time, writer.local_time());

  expect_processor_output("0 s:(0,1 s)\n"
                          "1 s:(4711,1 s)\n");
}

// ... even after the duration of the offset push
TEST_F(StreamStateSemantics, PushOffsetAndCommitMore)
{
  writer.push(dur, 4711, dur);
  writer.commit(3 * dur);
  expect_processor_output("0 s:(0,1 s)\n"
                          "1 s:(4711,1 s)\n"
                          "2 s:(0,1 s)\n");
}

TEST_F(StreamStateSemantics, PushOffsetAndTwoCommits)
{
  writer.push(dur, 4711, dur);
  writer.commit(dur);
  expect_processor_output("0 s:(0,1 s)\n");
  writer.commit(dur);
  expect_processor_output("1 s:(4711,1 s)\n");
}

// same but with an absolute commit time
TEST_F(StreamStateSemantics, PushOffsetAndTwoCommitsWithAbsoluteTime)
{
  writer.push(dur, 4711, dur);
  writer.commit(stamp);
  expect_processor_output("0 s:(0,1 s)\n");
  writer.commit(stamp);

  // the stream should not advance its time
  EXPECT_EQ(stamp, writer.end_time());
  expect_processor_output("");

  // committing with a duration should advance the stream again
  writer.commit(dur);

  EXPECT_EQ(stamp + dur, writer.end_time());
  expect_processor_output("1 s:(4711,1 s)\n");
}

// this is not allowed since it would lead to a merge
TEST_F(StreamStateSemantics, PushOffsetAndDurationAndCommitDeath)
{
  ASSERT_DEATH(
    {
      writer.push(dur, 4711, dur);
      writer.push(test_tuple);
    },
    "");
}

//-------------------------------------------------------------------

TEST_F(StreamStateSemantics, PushOffsetAndCommitFullDuration)
{
  writer.push(dur, 4711, dur);
  writer.commit(2 * dur);
  EXPECT_EQ(dur * 2, writer.end_time());

  expect_processor_output("0 s:(0,1 s)\n"
                          "1 s:(4711,1 s)\n");
}

TEST_F(StreamStateSemantics, PushIndefiniteTuples)
{
  writer.push(4711);

  writer.commit(dur);
  expect_processor_output("0 s:(4711,1 s)\n");

  // stream should keep the state if no new push was issued
  writer.commit(dur);
  expect_processor_output("1 s:(4711,1 s)\n");

  // check if we can push a new indefinite value
  writer.push(1337);

  writer.commit(dur * 3);
  expect_processor_output("2 s:(1337,3 s)\n");

  EXPECT_EQ(dur * 5, writer.end_time());

  // with no duration given, the stream should not advance any further
  writer.commit();
  EXPECT_EQ(dur * 5, writer.end_time());
}

TEST_F(StreamStateSemantics, PushZeroTimeAtEnd)
{
  writer.push(4711, dur);
  writer.push(4712, zero_time);
  writer.commit(dur);
  EXPECT_EQ(dur, writer.end_time());

  expect_processor_output("0 s:(4711,1 s)\n"
                          "1 s:(4712,0 s)\n");
}

TEST_F(StreamStateSemantics, PushZeroTimeAtEnd2)
{
  writer.push(4711, dur);
  writer.push(4712, zero_time);
  writer.push(4713, dur);
  writer.commit(dur * 2);
  EXPECT_EQ(dur * 2, writer.end_time());

  expect_processor_output("0 s:(4711,1 s)\n"
                          "1 s:(4712,0 s)\n"
                          "1 s:(4713,1 s)\n");
}

TEST_F(StreamStateSemantics, CheckFrontSplit)
{
  writer.push(0, dur * 2);
  writer.push(4711, zero_time);
  writer.push(0, dur * 122);
  writer.commit();

  auto result = reader.front(dur);

  EXPECT_EQ(result.duration(), dur);
}
