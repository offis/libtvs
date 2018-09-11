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

class StreamProcessSemantics
  : public timed_stream_fixture<double, tracing::timed_process_traits<double>>
{
protected:
  StreamProcessSemantics()
    : test_tuple(4711, dur)
  {
  }

  const tuple_type test_tuple;
};

// joining tuples with process behaviour should not result in a join
TEST_F(StreamProcessSemantics, JoinPolicy)
{
  writer.push(100, dur);
  writer.push(100, dur);
  writer.commit();
  expect_processor_output("0 s:(100,1 s)\n"
                          "1 s:(100,1 s)\n");
}

// splitting tuples with process behaviour should result in reduced values of
// the new tuples
TEST_F(StreamProcessSemantics, SplitPolicy)
{
  writer.push(100, dur * 2);
  writer.commit(dur);
  expect_processor_output("0 s:(50,1 s)\n");
  writer.commit(dur);
  expect_processor_output("1 s:(50,1 s)\n");
}

// check front() split policy
TEST_F(StreamProcessSemantics, SplitPolicyFront)
{
  writer.push(100, dur * 2);
  writer.commit();

  auto front = reader.front(dur * 2);
  EXPECT_EQ(front.duration(), dur * 2);

  reader.pop();

  writer.push(100, dur * 2);
  writer.commit();

  front = reader.front(dur);
  EXPECT_EQ(front.duration(), dur);
}


// checking local time advances
TEST_F(StreamProcessSemantics, LocalTimeAdvances)
{
  writer.push(100, dur * 2);
  writer.commit(dur);
  expect_processor_output("0 s:(50,1 s)\n");

  // the following push will append the value according to the stream's local
  // time offset, which is still in the 'future'
  writer.push(100, dur);

  writer.commit(dur);
  expect_processor_output("1 s:(50,1 s)\n");

  // the implicit commit with no time will pull all values out of the
  // stream
  writer.commit();
  expect_processor_output("2 s:(100,1 s)\n");
}

// merging tuples with process behaviour
TEST_F(StreamProcessSemantics, MergePolicy)
{
  writer.push(zero_time, 100, dur * 2);
  writer.commit(dur);
  expect_processor_output("0 s:(50,1 s)\n");
  writer.push(100, dur);
  writer.commit(dur);
  expect_processor_output("1 s:(150,1 s)\n");
  writer.commit(dur);
  expect_processor_output("2 s:(0,1 s)\n");
}

TEST_F(StreamProcessSemantics, PushToAdvanceAndFuture)
{

  writer.push(100, dur * 2);

  // local time offset has been advanced
  EXPECT_EQ(dur * 2, writer.duration());

  writer.push(zero_time, 100, dur * 2);
  writer.push(dur, 100, dur * 2);

  // local time offset has not been advanced
  EXPECT_EQ(dur * 2, writer.duration());

  writer.commit(dur * 6);
  expect_processor_output("0 s:(100,2 s)\n"
                          "2 s:(50,1 s)\n"
                          "3 s:(100,1 s)\n"
                          "4 s:(50,1 s)\n"
                          "5 s:(0,1 s)\n");
}

TEST_F(StreamProcessSemantics, PushOffsetAndPushToAdvance)
{
  writer.push(zero_time, 100, dur * 2);
  writer.push(dur, 100, dur * 2);

  // local time offset has not been advanced
  EXPECT_EQ(zero_time, writer.duration());

  writer.push(100, dur * 2);

  // local time offset has been advanced
  EXPECT_EQ(dur * 2, writer.duration());

  writer.commit(dur * 3);
  expect_processor_output("0 s:(100,1 s)\n"
                          "1 s:(150,1 s)\n"
                          "2 s:(50,1 s)\n");
}

TEST_F(StreamProcessSemantics, PushOffsetSmallDuration)
{
  writer.push(100, dur * 2);

  // local time offset has been advanced
  EXPECT_EQ(dur * 2, writer.duration());

  writer.push(zero_time, 100, dur * 2);
  writer.push(dur, 100, dur * 2);

  // local time offset has not been advanced
  EXPECT_EQ(dur * 2, writer.duration());

  writer.commit(dur * 6);
  expect_processor_output("0 s:(100,2 s)\n"
                          "2 s:(50,1 s)\n"
                          "3 s:(100,1 s)\n"
                          "4 s:(50,1 s)\n"
                          "5 s:(0,1 s)\n");
}

TEST_F(StreamProcessSemantics, PushDuration)
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
TEST_F(StreamProcessSemantics, PushOffsetAndCommitWithoutDuration)
{
  writer.push(dur, 4711, dur);
  EXPECT_EQ(zero_time, writer.local_time());

  writer.commit();
  EXPECT_EQ(zero_time, writer.end_time());
  expect_processor_output("");
}

// same for indefinite pushes
TEST_F(StreamProcessSemantics, PushIndefiniteAndCommitWithoutDuration)
{
  writer.push(4711);
  EXPECT_EQ(zero_time, writer.end_time());

  writer.commit();
  EXPECT_EQ(zero_time, writer.end_time());
  expect_processor_output("");
}

// Pushing with an offset followed by a commit should fill up the empty interval
// as defined by the empty_policy
TEST_F(StreamProcessSemantics, PushOffset)
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
TEST_F(StreamProcessSemantics, PushOffsetAndCommitMore)
{
  writer.push(dur, 4711, dur);
  writer.commit(3 * dur);
  expect_processor_output("0 s:(0,1 s)\n"
                          "1 s:(4711,1 s)\n"
                          "2 s:(0,1 s)\n");
}

TEST_F(StreamProcessSemantics, PushOffsetAndTwoCommits)
{
  writer.push(dur, 4711, dur);
  writer.commit(dur);
  expect_processor_output("0 s:(0,1 s)\n");
  writer.commit(dur);
  expect_processor_output("1 s:(4711,1 s)\n");
}

// same but with an absolute commit time
TEST_F(StreamProcessSemantics, PushOffsetAndTwoCommitsWithAbsoluteTime)
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

// Pushing a tuple should advance the stream's local time offset, even in the
// presence of future values
TEST_F(StreamProcessSemantics, PushOffsetAndDurationAndCommit)
{
  writer.push(dur, 4711, dur);
  writer.push(test_tuple);
  EXPECT_EQ(dur, writer.end_time());
}

//-------------------------------------------------------------------

TEST_F(StreamProcessSemantics, PushOffsetAndCommitFullDuration)
{
  writer.push(dur, 4711, dur);
  writer.commit(2 * dur);
  EXPECT_EQ(dur * 2, writer.end_time());

  expect_processor_output("0 s:(0,1 s)\n"
                          "1 s:(4711,1 s)\n");
}

// this is FORBIDDEN, it does not make sense for process-based values, as we
// define a process-based value to have infinite duration.
TEST_F(StreamProcessSemantics, PushIndefiniteTuplesDeath)
{
  ASSERT_DEATH(
               {
                 // error: pushing an indefinite value on a process-based stream
                 writer.push(4711);
                 writer.commit(dur);
               },
               "");
}
