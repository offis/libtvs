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
 * \file   timed_sequence.tpp
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  sequence of timed values (template implementation)
 * \see    timed_value.h
 *
 * This is an internal class used by the \ref timed_stream implementation
 * to hold an intermediate sequence of \ref timed_value instances.
 */

#include <tvs/tracing/timed_ranges.h>
#include <tvs/tracing/timed_sequence.h>
#include <tvs/tracing/timed_stream_policies.h>

namespace tracing {

// -----------------------------------------------------------------------

namespace impl {

/// generic sequence push back/front implementation
template<typename SequenceType, typename JoinPolicy>
struct timed_sequence_do_push
{
  typedef SequenceType sequence_type;
  typedef typename sequence_type::storage_type storage_type;
  typedef typename sequence_type::duration_type duration_type;

  template<typename OtherSequenceType>
  static void back(sequence_type& this_,
                   storage_type& buf,
                   OtherSequenceType const& seq)
  {
    if (buf.empty()) {
      duration_type d = seq.duration(); // remember old duration for aliasing
      buf.insert(buf.end(), seq.begin(), seq.end());
      this_.add_duration(d);
    } else {
      // element-wise push
      this_.push_back(seq.begin(), seq.end());
    }
  }

  ///\todo add front pushing
};

// specialized version for join_policy_separate
template<typename SequenceType>
struct timed_sequence_do_push<
  SequenceType,
  timed_join_policy_separate<typename SequenceType::value_type>>
{
  typedef SequenceType sequence_type;

  typedef typename sequence_type::value_type value_type;
  typedef typename sequence_type::storage_type storage_type;
  typedef typename sequence_type::duration_type duration_type;

  typedef timed_join_policy_separate<value_type> join_policy;

  template<typename OtherSequenceType>
  static void back(sequence_type& this_,
                   storage_type& buf,
                   OtherSequenceType const& seq)
  {
    duration_type d = seq.duration(); // remember old duration for aliasing
    buf.insert(buf.end(), seq.begin(), seq.end());
    this_.add_duration(d);
  }

  ///\todo add front pushing
};

} // namespace impl

template<typename T, typename Traits>
template<typename SequenceType>
void
timed_sequence<T, Traits>::push_back(SequenceType const& seq)
{
  if (seq.empty())
    return;

  SYSX_ASSERT(empty() || !buf_.back().is_infinite());

  typedef impl::timed_sequence_do_push<SequenceType, join_policy> push_type;
  push_type::back(*this, buf_, seq);
}

template<typename T, typename Traits>
void
timed_sequence<T, Traits>::split(duration_type const& offset)
{
  if (offset.is_infinite()) {
      SYSX_REPORT_FATAL(sysx::report::plain_msg)
        << "Cannot split sequence at infinite offset";
    }
  SYSX_ASSERT(offset <= this->duration() &&
              "Cannot split beyond sequence duration.");

  // empty sequence split semantics
  if (this->empty())
    return;

  // check common case
  if (offset == front_duration() || offset == duration())
    return;

  auto srange = this->range(this->before(offset).duration(), offset);

  // is there already a split at the offset?
  if (srange.offset() == offset)
    return;

  tuple_type rhs = srange.back();

  // perform split of the last tuple (will shorten rhs)
  auto lhs = split_policy::split(rhs, offset - srange.offset());

  this_type seq;

  // push the rhs back (possibly infinite)
  seq.push_back(rhs);

  // push the lhs to front (avoid join)
  seq.push_front(lhs);

  // replace old range with the new sequence
  srange.replace(seq);
}

// -----------------------------------------------------------------------

template<typename T, typename Traits>
void
timed_sequence<T, Traits>::print(std::ostream& os) const
{
  os << "{" << this->duration() << "; ";
  if (begin() == end()) {
    os << "- }";
    return;
  }
  const_iterator it = begin();
  while (it != end())
    os << *it++;
  os << " }";
}

} // namespace tracing

/* Taf!
 */
