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

using test_stream = tracing::timed_stream<double, tracing::timed_process_traits>;

struct SequenceSemantics : public timed_stream_fixture<test_stream>
{

  void SetUp()
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

  // check merge result of s1 and s2 (including commutative feature)
  void expect_merge_result(sequence_type const& s1,
                           sequence_type const& s2,
                           std::string const& str)
  {
    sequence_type s1m = s1;
    sequence_type s2m = s2;

    s1m.merge(s2);
    s2m.merge(s1);

    expect_sequence(s1m, str);
    expect_sequence(s2m, str);
  }

  sequence_type seq;
};

TEST_F(SequenceSemantics, CheckFront)
{
  sequence_type seq1;
  seq1.push_front(0, dur);
  EXPECT_EQ(0, seq1.front());

  seq1.front(1, dur);
  EXPECT_EQ(1, seq1.front());

  seq1.front(2);
  EXPECT_EQ(2, seq1.front());

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
  EXPECT_EQ(0, seq1.back());

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


TEST_F(SequenceSemantics, CheckBeforeDuration)
{
  sequence_type::range_type range = seq.before(dur + dur);
  EXPECT_EQ(dur + dur, range.duration());
}

// before() returns tuples that are inside the given area
TEST_F(SequenceSemantics, CheckBeforeDurationNotCovering)
{
  sequence_type::range_type range = seq.before(dur * 1.5);
  EXPECT_EQ(dur, range.duration());
}

TEST_F(SequenceSemantics, CheckRangeDuration)
{
  sequence_type::range_type range = seq.range(dur + dur);
  EXPECT_EQ(dur + dur, range.duration());
}

// range() also returns tuples that are covered by the area
TEST_F(SequenceSemantics, CheckRangeDurationCovering)
{
  sequence_type::range_type range = seq.range(dur * 1.5);
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

//////////// CHECK MERGE SEMANTICS /////////////

TEST_F(SequenceSemantics, CheckMergeEmpty)
{
  sequence_type seq1, seq2;

  seq2.push_back(1, dur);
  seq2.push_back(0, dur);
  expect_merge_result(seq1, seq2, "{2 s; (1,1 s)(0,1 s) }");
}

TEST_F(SequenceSemantics, CheckMergeEmptyInfinite)
{
  sequence_type seq1, seq2;

  seq1.push_back(1, inf);
  expect_sequence(seq1, "{inf; (1,inf) }");

  expect_merge_result(seq1, seq2, "{inf; (1,inf) }");
}


// test a more complicated merge where splits should happen
TEST_F(SequenceSemantics, CheckMergeWithSplit)
{
  sequence_type seq1, seq2;

  seq1.push_back(1, dur/2);
  seq1.push_back(2, dur/2);
  seq1.push_back(3, dur*3);

  expect_sequence(seq1, "{4 s; (1,500 ms)(2,500 ms)(3,3 s) }");

  seq2.push_back(4, dur * 2);
  seq2.push_back(5, dur);
  seq2.push_back(6, dur);

  expect_sequence(seq2, "{4 s; (4,2 s)(5,1 s)(6,1 s) }");

  expect_merge_result(seq1, seq2, "{4 s; (2,500 ms)(3,500 ms)(3,1 s)(6,1 s)(7,1 s) }");
}
