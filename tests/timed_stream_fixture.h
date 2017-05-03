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

#ifndef TVS_TESTS_TIMED_STREAM_FIXTURE_H_INCLUDED_
#define TVS_TESTS_TIMED_STREAM_FIXTURE_H_INCLUDED_

#include "gtest/gtest.h"

#include "tvs/tracing.h"

/// base fixture class for providing convenience members which are used
/// throughout the tests.
struct timed_stream_fixture_b : public ::testing::Test
{

  /// absolute time stamp
  tracing::time_type stamp;

  tracing::timed_duration dur;
  tracing::time_type zero_time;
  tracing::timed_duration inf;

  timed_stream_fixture_b()
    : stamp(tracing::time_type(1, sc_core::SC_SEC))
    , dur(stamp)
    , zero_time(tracing::timed_duration::zero_time)
    , inf(tracing::timed_duration::infinity())
  {
  }
};

/// fixture class to provide typedefs used in the tests
template<typename Stream>
struct timed_stream_fixture : public timed_stream_fixture_b
{
  typedef Stream stream_type;

  typedef typename stream_type::tuple_type tuple_type;
  typedef typename stream_type::reader_type reader_type;
  typedef typename stream_type::writer_type writer_type;
  typedef typename stream_type::sequence_type sequence_type;
};

#endif // TVS_TESTS_TIMED_STREAM_FIXTURE_H_INCLUDED_
