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

#include <map>

/// Example usage of custom traits

enum class test_state
{
  NONE = 0,
  IDLE,
  RUNNING,
  BLOCKED
};

namespace sysx {
namespace utils {

// Provide variant support for our custom type
template<>
struct variant_traits<test_state> : variant_traits_disabled<test_state>
{
};

} // namespace utils
} // namespace sysx

// our custom state traits allow merging by accumulating, which will use the
// operator+ defined below.  The intent is to be able to merge states where one
// state is NONE.
struct my_traits : tracing::timed_state_traits<test_state>
{
  typedef tracing::timed_merge_policy_accumulate<test_state> merge_policy;
};

/// provide a way to print out the enum value
std::ostream&
operator<<(std::ostream& out, test_state const& e)
{
// clang-format off
#define _MAP(event) {test_state::event, #event}
  // clang-format on

  static const std::map<test_state, const char*> cached = {
    _MAP(NONE), _MAP(IDLE), _MAP(RUNNING), _MAP(BLOCKED),
  };

#undef _MAP

  return out << cached.at(e);
}

test_state
operator+(test_state const& lhs, test_state const& rhs)
{
  int result = static_cast<int>(lhs) + static_cast<int>(rhs);

  SYSX_ASSERT(lhs == test_state::NONE || rhs == test_state::NONE);

  return static_cast<test_state>(result);
}

struct CustomTraitsSemantics
  : public timed_stream_fixture<test_state, my_traits>
{
};

//////////// CHECK MERGE SEMANTICS /////////////

TEST_F(CustomTraitsSemantics, CheckJoin)
{

  writer.push(test_state::NONE, dur);
  writer.push(test_state::BLOCKED, dur);
  writer.commit();
  expect_processor_output("0 s:(NONE,1 s)\n"
                          "1 s:(BLOCKED,1 s)\n");

  writer.push(test_state::IDLE, dur);
  writer.commit();
  expect_processor_output("2 s:(IDLE,1 s)\n");
  writer.push(test_state::NONE, dur);
  writer.push(test_state::NONE, dur);
  writer.commit();
  expect_processor_output("3 s:(NONE,2 s)\n");
}

TEST_F(CustomTraitsSemantics, CheckSplitMerge)
{
  writer.push(zero_time, test_state::NONE, dur * 2);
  writer.push(dur * 2, test_state::BLOCKED, dur);
  writer.push(zero_time, test_state::BLOCKED, dur);
  writer.push(dur, test_state::NONE, dur * 2);
  writer.commit(dur * 3);

  expect_processor_output("0 s:(BLOCKED,1 s)\n"
                          "1 s:(NONE,1 s)\n"
                          "2 s:(BLOCKED,1 s)\n");

  writer.push(zero_time, test_state::NONE, dur);
  writer.push(zero_time, test_state::BLOCKED, dur * 2);
  writer.commit(3 * dur);

  expect_processor_output("3 s:(BLOCKED,2 s)\n"
                          "5 s:(NONE,1 s)\n");
}

TEST_F(CustomTraitsSemantics, CheckMergeSingleInfinite)
{
  writer.push(test_state::RUNNING);
  writer.push(zero_time, test_state::NONE, dur);
  writer.commit(3 * dur);

  // here both tuples are 'added' so we only see RUNNING
  expect_processor_output("0 s:(RUNNING,3 s)\n");
}

TEST_F(CustomTraitsSemantics, CheckMergeBothInfinite)
{
  writer.push(test_state::RUNNING);
  writer.push(test_state::NONE);
  writer.commit(3 * dur);

  // here the second call overwrites the inf tuple value in the stream
  expect_processor_output("0 s:(NONE,3 s)\n");
}
