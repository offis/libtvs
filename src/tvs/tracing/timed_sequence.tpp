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
 * \file   timed_sequence.tpp
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  sequence of timed values (template implementation)
 * \see    timed_value.h
 *
 * This is an internal class used by the \ref timed_stream implementation
 * to hold an intermediate sequence of \ref timed_value instances.
 */

#include "tvs/tracing/timed_ranges.h"
#include "tvs/tracing/timed_sequence.h"
#include "tvs/tracing/timed_stream_policies.h" // JoinPolicy

namespace tracing {

// -----------------------------------------------------------------------

namespace impl {

/// generic sequence merge implementation
template<typename SequenceType, typename EmptyPolicy>
struct timed_sequence_do_merge
{
  typedef SequenceType sequence_type;

  typedef typename sequence_type::tuple_type tuple_type;
  typedef typename sequence_type::split_policy split_policy;
  typedef typename sequence_type::merge_policy merge_policy;

  template<typename OtherSequenceType>
  static void merge(sequence_type& this_, OtherSequenceType&& other)
  {
    if (other.empty())
      return;

    if (this_.empty()) {
      this_.move_back(other);
      return;
    }

    sequence_type result;
    sequence_type *seq_a = &this_, *seq_b = &other;

    // iterate until one sequence is empty
    while (!seq_a->empty() && !seq_b->empty()) {

      // make sure seq_a points to the sequence with the longer front tuple
      if (seq_a->front_duration() < seq_b->front_duration())
        std::swap(seq_a, seq_b);

      // split the front tuple of A, if necessary
      tuple_type a_front = seq_a->front();
      if (seq_a->front_duration() > seq_b->front_duration()) {
        tuple_type rhs = a_front;
        // split and consume rhs of split
        a_front = split_policy::split(rhs, seq_b->front_duration());
        seq_a->front(rhs);
      } else {
        // nothing to split, we can consume the whole tuple
        seq_a->pop_front();
      }

      merge_policy::merge(a_front, seq_b->front());
      result.push_back(a_front);
      seq_b->pop_front();
    }

    // handle leftover tuples in both sequences
    if (!seq_a->empty())
      result.move_back(*seq_a);
    if (!seq_b->empty())
      result.move_back(*seq_b);

    this_.clear();
    this_.move_back(result);
  }
};

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

template<typename T, template<typename> class U>
template<typename SequenceType>
void
timed_sequence<T, U>::push_back(SequenceType const& seq)
{
  if (seq.empty())
    return;

  SYSX_ASSERT(empty() || !buf_.back().is_infinite());

  typedef impl::timed_sequence_do_push<SequenceType, join_policy> push_type;
  push_type::back(*this, buf_, seq);
}

template<typename T, template<typename> class U>
template<typename SequenceType>
void
timed_sequence<T, U>::merge(SequenceType seq)
{
  typedef impl::timed_sequence_do_merge<SequenceType, empty_policy> merge_type;
  merge_type::merge(*this, std::move(seq));
}

template<typename T, template<typename> class U>
void
timed_sequence<T, U>::split(duration_type const& offset, bool extend)
{
  auto srange = this->range(offset);

  if (srange.duration() == offset)
    return;

  if (srange.duration() < offset) {
    if (extend)
      this->push_back(empty_policy::empty(offset - srange.duration()));
    return;
  }

  // we need to split the last tuple in the srange
  this_type tmp;
  tmp.push_back(srange.begin(), srange.end());

  tuple_type rhs = tmp.back();
  duration_type split = offset - this->before(offset).duration();

  this->pop_front(srange.duration());

  if (rhs.is_infinite()) {
    tmp.back(rhs.value(), split);
  } else {
    tmp.back(split_policy::split(rhs, split));
  }

  tmp.push_back(rhs, /* join = */ false);

  // append remaining sequence
  tmp.push_back(*this);
  this->swap(tmp);
}

// -----------------------------------------------------------------------

template<typename T, template<typename> class U>
void
timed_sequence<T, U>::print(std::ostream& os) const
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
