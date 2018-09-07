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

#include "timed_stream_fixture.h"

#include "tvs/tracing.h"

#include "gtest/gtest.h"

struct SequenceSemantics
  : public timed_stream_fixture<double, tracing::timed_process_traits<double>>
{

  void SetUp() override
  {
    seq.push_back(0, dur);
    seq.push_back(1, dur);
    seq.push_back(2, dur);
  }

  void expect_sequence(sequence_type const& s, std::string const& str)
  {
    std::stringstream strs;
    strs << s;
    EXPECT_EQ(str, strs.str());
  }

  sequence_type seq;
};

TEST_F(SequenceSemantics, CheckFront)
{
  sequence_type seq1;
  seq1.push_front(0, dur);
  EXPECT_EQ(0, seq1.front().value());

  seq1.front(1, dur);
  EXPECT_EQ(1, seq1.front().value());

  seq1.front(2);
  EXPECT_EQ(2, seq1.front().value());

  tuple_type tup(1, dur * 2);
  seq1.front(tup);
  EXPECT_EQ(tup, seq1.front());

  seq1.pop_front();
  EXPECT_TRUE(seq1.empty());
}

TEST_F(SequenceSemantics, CheckBack)
{
  sequence_type seq1;
  seq1.push_back(0, dur);
  EXPECT_EQ(0, seq1.back().value());

  tuple_type tup(1, dur * 2);
  seq1.back(tup);
  EXPECT_EQ(tup, seq1.back());

  tuple_type tup2(2, dur);
  seq1.back(2, dur);
  EXPECT_EQ(tup2, seq1.back());

  seq1.pop_back();
  EXPECT_TRUE(seq1.empty());

  seq1.push_back(tup);
  seq1.push_back(2, inf);

  EXPECT_TRUE(seq1.back().is_infinite());

  seq1.pop_back();
  EXPECT_EQ(tup, seq1.back());
}

/// check if a range covers the correct elements
TEST_F(SequenceSemantics, RangeSemantics)
{
  auto range = seq.range(dur, dur * 3);
  EXPECT_EQ(range.duration(), dur * 2);

  EXPECT_EQ(range.front(), *(++seq.begin()));
  EXPECT_EQ(range.back(), *(--seq.end()));

  tuple_type tup(1234, dur);

  range.front(tup);

  EXPECT_EQ(range.front(), tup);
}

/// check range front modifications
TEST_F(SequenceSemantics, RangeFrontSemantics)
{
  auto range = seq.range(dur * 2);
  EXPECT_EQ(range.duration(), dur * 2);

  EXPECT_EQ(range.front(), seq.front());
  tuple_type tup(1234, dur);

  range.front(tup);
  EXPECT_EQ(range.front(), tup);

  range.front(123, dur * 1.2);
  EXPECT_EQ(range.front(), tuple_type(123, dur * 1.2));

  range.front(125);
  EXPECT_EQ(range.front(), tuple_type(125, dur * 1.2));
}

/// check if a range back modifications
TEST_F(SequenceSemantics, RangeBackSemantics)
{
  auto range = seq.range(dur * 2);
  EXPECT_EQ(range.duration(), dur * 2);

  EXPECT_EQ(range.front(), tuple_type(0, dur));
  EXPECT_EQ(range.back(), tuple_type(1, dur));

  range.back(123, dur * 1.2);
  EXPECT_EQ(range.back(), tuple_type(123, dur * 1.2));
  EXPECT_EQ(range.duration(), dur + dur * 1.2);

  // check if the whole sequence has been updated
  EXPECT_EQ(seq.duration(), dur + dur * 1.2 + dur);

  range.back(125);
  EXPECT_EQ(range.back(), tuple_type(125, dur * 1.2));
}

TEST_F(SequenceSemantics, CheckBeforeDuration)
{
  sequence_type::range_type range = seq.before(dur + dur);
  EXPECT_EQ(dur + dur, range.duration());
}

TEST_F(SequenceSemantics, CheckRangeAndBeforeDurations)
{
  // before() returns tuples that are inside the given duration
  sequence_type::range_type range = seq.before(dur * 1.5);
  EXPECT_EQ(dur, range.duration());

  // range() additionally returns tuples that are covered by the duration
  sequence_type::range_type range2 = seq.range(dur * 1.5);
  EXPECT_EQ(2 * dur, range2.duration());

  // this also works for the start tupls
  sequence_type::range_type range3 = seq.range(dur * 0.5, dur * 1.5);
  EXPECT_EQ(2 * dur, range3.duration());
}

TEST_F(SequenceSemantics, CheckRangeDuration)
{
  sequence_type::range_type range = seq.range(dur + dur);
  EXPECT_EQ(dur + dur, range.duration());
}

TEST_F(SequenceSemantics, CheckSequencePush)
{
  sequence_type::range_type range = seq.range(dur);
  sequence_type seq2;
  seq2.push_back(range.begin(), range.end());
  EXPECT_EQ(dur, seq2.duration());
}

TEST_F(SequenceSemantics, CheckSequenceZeroTimeRangePush)
{
  sequence_type::range_type range = seq.range(zero_time);
  sequence_type seq2;
  seq2.push_back(range.begin(), range.end());
  EXPECT_EQ(zero_time, seq2.duration());
  EXPECT_EQ(true, seq2.empty());
}

TEST_F(SequenceSemantics, CheckIfMoveOnSequence)
{
  sequence_type seq1, seq2;
  seq1.push_back(0, dur);
  seq2.push_back(2, inf);

  seq1.move_back(seq2);

  expect_sequence(seq1, "{inf; (0,1 s)(2,inf) }");
}

TEST_F(SequenceSemantics, CheckInfPushOnSequence)
{
  sequence_type seq1, seq2;
  seq1.push_back(0, dur);
  seq2.push_back(2, inf);

  seq1.push_back(seq2);

  expect_sequence(seq1, "{inf; (0,1 s)(2,inf) }");
}

TEST_F(SequenceSemantics, CheckSplitSemantics)
{
  // Don't do anything if the split exists
  seq.split(zero_time);
  expect_sequence(seq, "{3 s; (0,1 s)(1,1 s)(2,1 s) }");

  seq.split(dur);
  expect_sequence(seq, "{3 s; (0,1 s)(1,1 s)(2,1 s) }");

  seq.split(dur * 2);
  expect_sequence(seq, "{3 s; (0,1 s)(1,1 s)(2,1 s) }");

  seq.split(seq.duration());
  expect_sequence(seq, "{3 s; (0,1 s)(1,1 s)(2,1 s) }");

  seq.split(dur * 1.2);

  sequence_type exp;

  exp.push_back(0, dur);
  exp.push_back(0.2, dur * (1.0 / 5.0));
  exp.push_back(0.8, dur * (4.0 / 5.0));
  exp.push_back(2, dur);

  std::stringstream sstr;

  sstr << exp;

  expect_sequence(seq, sstr.str());

  ASSERT_DEATH({ seq.split(inf); }, "");
}
