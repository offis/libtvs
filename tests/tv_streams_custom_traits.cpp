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

/// Example usage of custom traits

enum class test_state
{
  NONE = 0,
  IDLE,
  RUNNING,
  BLOCKED
};

// our custom state traits allow merging by accumulating, which will use the
// operator+ defined below.  The intent is to be able to merge states where one
// state is NONE.
template<typename T>
struct my_traits : tracing::timed_state_traits<T>
{
  typedef tracing::timed_merge_policy_accumulate<T> merge_policy;
};

/// provide a way to print out the enum value
std::ostream&
operator<<(std::ostream& out, test_state const& e)
{
  typedef test_state event_type;

#define _MAP(event)                                                            \
  case event_type::event:                                                      \
    return out << #event;
  switch (e) {
    _MAP(NONE);
    _MAP(IDLE);
    _MAP(RUNNING);
    _MAP(BLOCKED);
  }
#undef _MAP

  return out;
}

test_state
operator+(test_state const& lhs, test_state const& rhs)
{
  int result = static_cast<int>(lhs) + static_cast<int>(rhs);

  SYSX_ASSERT(lhs == test_state::NONE || rhs == test_state::NONE);

  return static_cast<test_state>(result);
}

using test_stream =
                  tracing::timed_stream<test_state, my_traits>;

struct CustomTraitsSemantics : public timed_stream_fixture<test_stream>
{

  void SetUp()
  {
    seq.push_back(test_state::IDLE, dur);
    seq.push_back(test_state::RUNNING, dur);
    seq.push_back(test_state::BLOCKED, dur);
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

//////////// CHECK MERGE SEMANTICS /////////////

TEST_F(CustomTraitsSemantics, CheckJoin)
{
  sequence_type seq1, seq2;

  seq1.push_back(test_state::NONE, dur);
  seq2.push_back(test_state::BLOCKED, dur);
  expect_merge_result(seq1, seq2, "{1 s; (BLOCKED,1 s) }");

  seq2.push_back(test_state::IDLE, dur);
  expect_merge_result(seq1, seq2, "{2 s; (BLOCKED,1 s)(IDLE,1 s) }");
}

TEST_F(CustomTraitsSemantics, CheckSplitMerge)
{
  sequence_type seq1, seq2;

  seq1.push_back(test_state::NONE, dur * 2);
  seq1.push_back(test_state::BLOCKED, dur);
  expect_sequence(seq1, "{3 s; (NONE,2 s)(BLOCKED,1 s) }");

  seq2.push_back(test_state::BLOCKED, dur);
  seq2.push_back(test_state::NONE, dur * 2);
  expect_sequence(seq2, "{3 s; (BLOCKED,1 s)(NONE,2 s) }");

  expect_merge_result(
    seq1, seq2, "{3 s; (BLOCKED,1 s)(NONE,1 s)(BLOCKED,1 s) }");

  seq1.push_back(test_state::NONE, dur);
  seq2.push_back(test_state::BLOCKED, dur * 2);
  expect_merge_result(
    seq1, seq2, "{5 s; (BLOCKED,1 s)(NONE,1 s)(BLOCKED,3 s) }");
}

TEST_F(CustomTraitsSemantics, CheckMergeEmptyInfinite)
{
  sequence_type seq1, seq2;

  seq1.push_back(test_state::RUNNING, inf);
  expect_sequence(seq1, "{inf; (RUNNING,inf) }");

  expect_merge_result(seq1, seq2, "{inf; (RUNNING,inf) }");
}

TEST_F(CustomTraitsSemantics, CheckMergeSingleInfinite)
{
  sequence_type seq1, seq2;

  seq1.push_back(test_state::RUNNING, inf);
  expect_sequence(seq1, "{inf; (RUNNING,inf) }");

  seq2.push_back(test_state::NONE, dur);
  expect_sequence(seq2, "{1 s; (NONE,1 s) }");

  expect_merge_result(seq1, seq2, "{inf; (RUNNING,inf) }");
}

TEST_F(CustomTraitsSemantics, CheckMergeBothInfinite)
{
  sequence_type seq1, seq2;

  seq1.push_back(test_state::RUNNING, inf);
  expect_sequence(seq1, "{inf; (RUNNING,inf) }");

  seq2.push_back(test_state::NONE, inf);
  expect_sequence(seq2, "{inf; (NONE,inf) }");

  expect_merge_result(seq1, seq2, "{inf; (RUNNING,inf) }");
}

TEST_F(CustomTraitsSemantics, CheckMergeBothInfiniteSplit)
{
  sequence_type seq1, seq2;
  seq1.push_back(test_state::NONE, dur * 2);
  seq1.push_back(test_state::RUNNING, inf);

  seq2.push_back(test_state::IDLE, dur);
  seq2.push_back(test_state::NONE, inf);

  expect_merge_result(
    seq1, seq2, "{inf; (IDLE,1 s)(NONE,1 s)(RUNNING,inf) }");
}

