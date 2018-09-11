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

/**
 * \file   timed_stream_traits.h
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>
 * \brief  Timed stream traits classes for process and state quantities
 * \see    timed_stream.h
 */
#ifndef TVS_TIMED_STREAM_TRAITS_H_INCLUDED_
#define TVS_TIMED_STREAM_TRAITS_H_INCLUDED_

#include <tvs/tracing/timed_stream_policies.h>

namespace tracing {

template<typename T>
struct timed_process_traits
{
  typedef T value_type;
  typedef timed_empty_policy_silence<value_type> empty_policy;
  typedef timed_split_policy_average<value_type> split_policy;
  typedef timed_join_policy_separate<value_type> join_policy;
  typedef timed_merge_policy_accumulate<value_type> merge_policy;
};

template<typename T>
struct timed_state_traits
{
  typedef T value_type;
  typedef timed_empty_policy_default<value_type> empty_policy;
  typedef timed_split_policy_keep<value_type> split_policy;
  typedef timed_join_policy_combine<value_type> join_policy;
  typedef timed_merge_policy_error<value_type> merge_policy;
};

template<typename T>
struct timed_event_traits
{
  typedef T value_type;
  typedef timed_empty_policy_default<value_type> empty_policy;
  typedef timed_split_policy_decay<value_type> split_policy;
  typedef timed_join_policy_separate<value_type> join_policy;
  typedef timed_merge_policy_union<value_type> merge_policy;
};

} // namespace tracing

#endif // TVS_TIMED_STREAM_TRAITS_H_INCLUDED_
/* Taf!
 */
