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

/**
 * \file   timed_stream_policies.tpp
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  split/merge/join policies for tracing streams (template
 * implementation)
 * \see    timed_stream.h
 */

#include <tvs/tracing/timed_value.h>

namespace tracing {

/* --------------------------- split policies -------------------------- */

template<typename T>
struct timed_split_policy_keep
{
  typedef T value_type;
  typedef timed_value<T> tuple_type;
  typedef typename tuple_type::duration_type duration_type;

  static tuple_type split(tuple_type& old, duration_type const& split_at)
  {
    SYSX_ASSERT(split_at < old.duration());

    if (!old.is_infinite())
      old.duration(old.duration() - split_at);

    // keep old value - no value "split"
    return tuple_type(old.value(), split_at);
  }
};

template<typename T>
struct timed_split_policy_average
{
  typedef T value_type;
  typedef timed_value<T> tuple_type;
  typedef typename tuple_type::duration_type duration_type;

  static tuple_type split(tuple_type& old, duration_type const& split_at)
  {
    if (old.is_infinite()) {
      SYSX_REPORT_FATAL(sysx::report::plain_msg)
        << "This policy cannot split infinite tuples";
    }
    SYSX_ASSERT(split_at < old.duration());

    using utype = sysx::units::time_type;

    const duration_type remainder = old.duration() - split_at;

    tuple_type result(old.value() * utype(split_at) / utype(old.duration()),
                      split_at);
    old = tuple_type(old.value() * utype(remainder) / utype(old.duration()),
                     remainder);

    return result;
  }
};

template<typename T>
struct timed_split_policy_decay
{
  typedef T value_type;
  typedef timed_value<T> tuple_type;
  typedef typename tuple_type::duration_type duration_type;

  /// split the \a old tuple at \a split_at by reducing the duration of \a old
  /// and returning a new default value as the lhs.
  static tuple_type split(tuple_type& old, duration_type const& split_at)
  {
    SYSX_ASSERT(split_at < old.duration());

    if (!old.is_infinite())
      old.duration(old.duration() - split_at);

    // default value for new tuple
    return tuple_type(value_type(), split_at);
  }
};

/* --------------------------- merge policies -------------------------- */

template<typename T>
struct timed_merge_policy_error
{
  typedef T value_type;
  typedef timed_value<T> tuple_type;
  typedef typename tuple_type::duration_type duration_type;

  static void merge(tuple_type&, tuple_type const&)
  {
    SYSX_REPORT_FATAL(sysx::report::plain_msg)
      << "Merge forbidden by policy";
  }
};

template<typename T>
struct timed_merge_policy_accumulate
{
  typedef T value_type;
  typedef timed_value<T> tuple_type;
  typedef typename tuple_type::duration_type duration_type;

  static void merge(tuple_type& back, tuple_type const& other)
  {
    SYSX_ASSERT(back.duration() == other.duration());
    back.value(back.value() + other.value());
  }
};

template<typename T>
struct timed_merge_policy_union
{
  typedef T value_type;
  typedef timed_value<T> tuple_type;
  typedef typename tuple_type::duration_type duration_type;

  static void merge(tuple_type& back, tuple_type const& other)
  {
    SYSX_ASSERT(back.duration() == other.duration());
    auto const& rhs = other.value();
    back.value().insert(rhs.begin(), rhs.end());
  }
};

template<typename T>
struct timed_merge_policy_average;

template<typename T>
struct timed_merge_policy_maximum;

template<typename T>
struct timed_merge_policy_override;

/* --------------------------- join policies -------------------------- */

template<typename T>
struct timed_join_policy_combine
{
  typedef T value_type;
  typedef timed_value<value_type> tuple_type;

  static bool join(tuple_type& back, tuple_type const& join)
  {
    if (back.value() == join.value()) {
      back.add_duration(join.duration());
      return true;
    }
    return false;
  }
};

template<typename T>
struct timed_join_policy_separate
{
  typedef T value_type;
  typedef timed_value<value_type> tuple_type;

  static bool join(tuple_type&, tuple_type const&) { return false; }
};

/* --------------------------- empty policies -------------------------- */

template<typename T>
struct timed_empty_policy_default
{
  typedef T value_type;
  typedef timed_value<value_type> tuple_type;

  static tuple_type empty(typename tuple_type::duration_type const& dur)
  {
    auto ret = tuple_type();
    ret.duration(dur);
    return ret;
  }
};

template<typename T>
struct timed_empty_policy_silence
{
  typedef T value_type;
  typedef timed_value<value_type> tuple_type;

  static tuple_type empty(typename tuple_type::duration_type const& dur)
  {
    // FIXME: Use variant to represent a silent value?
    return tuple_type(value_type(), dur);
  }
};

/* --------------------------------------------------------------------- */

} // namespace tracing

/* Taf!
 */
