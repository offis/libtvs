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
 * \file   timed_stream_policies.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  split/merge policies for tracing streams
 * \see    timed_stream.h
 */
#ifndef TVS_TIMED_STREAM_POLICIES_H_INCLUDED_
#define TVS_TIMED_STREAM_POLICIES_H_INCLUDED_

#include <tvs/tracing/timed_value.h>

namespace tracing {

/* --------------------------- split policies -------------------------- */

template<typename T>
struct timed_split_policy_keep;

template<typename T>
struct timed_split_policy_average;

template<typename T>
struct timed_split_policy_decay;

/* --------------------------- merge policies -------------------------- */

template<typename T>
struct timed_merge_policy_error;

template<typename T>
struct timed_merge_policy_accumulate;

template<typename T>
struct timed_merge_policy_average;

template<typename T>
struct timed_merge_policy_maximum;

template<typename T>
struct timed_merge_policy_override;

template<typename T>
struct timed_merge_policy_union;

/* --------------------------- join policies -------------------------- */

template<typename T>
struct timed_join_policy_combine;

template<typename T>
struct timed_join_policy_separate;

/* --------------------------- empty policies -------------------------- */

template<typename T>
struct timed_empty_policy_default;

template<typename T>
struct timed_empty_policy_silence;

/* ------------------------- zero time policies ------------------------ */

template<typename T>
struct timed_zero_time_policy_absorb;

template<typename T>
struct timed_zero_time_policy_keep;

/* --------------------------------------------------------------------- */

} // namespace tracing

#include <tvs/tracing/timed_stream_policies.tpp>

#endif /* TVS_TIMED_STREAM_POLICIES_H_INCLUDED_ */
/* Taf!
 */
