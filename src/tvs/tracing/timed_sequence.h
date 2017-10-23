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
 * \file   timed_sequence.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  sequence of timed values
 * \see    timed_value.h
 *
 * This is an internal class used by the \ref timed_stream implementation
 * to hold an intermediate sequence of \ref timed_value instances.
 */

#ifndef TVS_TIMED_SEQUENCE_H_INCLUDED_
#define TVS_TIMED_SEQUENCE_H_INCLUDED_

#include "tvs/tracing/timed_value.h"

#include <deque>

namespace tracing {

// -----------------------------------------------------------------------
// forward declarations
template<typename, typename>
class const_timed_range;
template<typename, typename>
class timed_range;

namespace impl {
template<typename, typename>
struct timed_sequence_do_push;
} // namespace impl

/// base class for timed sequences
class timed_sequence_base
{
  template<typename, typename>
  friend struct impl::timed_sequence_do_push;

public:
  typedef tracing::time_type time_type;
  typedef tracing::timed_duration duration_type;

  /// currently held duration of the sequence
  duration_type duration() const { return duration_; }

  /// infinite sequence?
  bool is_infinite() const { return duration_.is_infinite(); }

protected:
  timed_sequence_base()
    : duration_()
  {}

  /// update duration (from concrete sequence)
  void set_duration(duration_type const& d) { duration_ = d; }

  /// extend duration (from concrete sequence)
  void add_duration(duration_type const& d) { duration_ += d; }

  /// shrink duration (from concrete sequence)
  void del_duration(duration_type const& d)
  {
    if (!duration_.is_infinite()) {
      SYSX_ASSERT(d <= duration_);
      duration_ -= d;
    } else if (d.is_infinite()) {
      duration_ = duration_type::zero_time;
    } else {
      SYSX_ASSERT(!d.is_infinite());
    }
  }

  duration_type duration_;
};

template<typename T, typename Traits>
class timed_sequence
  : public timed_sequence_base
  , protected Traits::join_policy
  , protected Traits::split_policy
{
  friend class const_timed_range<T, Traits>;
  friend class timed_range<T, Traits>;
  template<typename, typename>
  friend struct impl::timed_sequence_do_push;

public:
  // ---------------------------------------------------------------------
  typedef timed_sequence_base base_type;
  typedef timed_sequence this_type;
  typedef base_type::time_type time_type;
  typedef base_type::duration_type duration_type;

  typedef T value_type;
  typedef timed_value<T> tuple_type;
  typedef std::deque<tuple_type> storage_type;

  typedef Traits traits_type;

  typedef typename traits_type::join_policy join_policy;
  typedef typename traits_type::split_policy split_policy;

  typedef typename storage_type::size_type size_type;
  typedef typename storage_type::iterator iterator;
  typedef typename storage_type::const_iterator const_iterator;
  typedef typename storage_type::reference reference;
  typedef typename storage_type::const_reference const_reference;

  typedef timed_range<value_type, traits_type> range_type;
  typedef const_timed_range<value_type, traits_type> const_range_type;

  // ---------------------------------------------------------------------

  size_type size() const { return buf_.size(); }

  /// sequence is empty?
  bool empty() const { return size() == 0; }

  /// swap two sequences
  void swap(this_type& that)
  {
    buf_.swap(that.buf_);
    duration_.swap(that.duration_);
  }

  /// empty the sequence
  void clear()
  {
    buf_.clear();
    set_duration(duration_type());
  }

  // ---------------------------------------------------------------------
  /** \name append to the timed_sequence */
  ///\{

  /// append a (value,duration) pair
  void push_back(value_type const& v, duration_type const& d)
  {
    push_back(tuple_type(v, d));
  }

  /// append a tuple
  void push_back(tuple_type const& t, bool join = true)
  {
    SYSX_ASSERT(empty() || !buf_.back().is_infinite());

    if (empty() || !(join && join_policy::join(back(), t)))
      buf_.push_back(t);
    add_duration(t.duration());
  }

  /// append a range
  template<typename InputIterator>
  void push_back(InputIterator from, InputIterator to)
  {
    while (from != to)
      push_back(*from++);
  }

  /// append another sequence
  template<typename SequenceType>
  void push_back(SequenceType const& seq);

  /// move contents of another sequence to the end of this one
  void move_back(this_type& seq)
  {
    if (empty()) {
      swap(seq);
      return;
    }
    push_back(seq);
    seq.clear();
  }
  ///\}

  // ---------------------------------------------------------------------
  /** \name access head of the sequence */
  ///\{

  /// read front of the sequence
  const_reference front() const { return buf_.front(); }

  /// update/replace (value of) first element in the sequence
  void front(value_type const& v) { buf_.front().value(v); }

  /// update/replace first element in the sequence
  void front(value_type const& v, duration_type const& d)
  {
    front(tuple_type(v, d));
  }

  /// update/replace first element in the sequence
  void front(tuple_type const& t)
  {
    SYSX_ASSERT(!t.is_infinite() || buf_.front().is_infinite());
    duration_type d = buf_.front().duration();
    buf_.front() = t;
    if (t.is_infinite()) {
      set_duration(duration_type::infinity());
    } else if (t.duration() < d) { // shorter tuple
      del_duration(d - t.duration());
    } else {
      add_duration(t.duration() - d);
    }
  }

  /// push an element to the front of the sequence
  void push_front(value_type const& v, duration_type const& d)
  {
    push_front(tuple_type(v, d));
  }

  /// push an element to the front of the sequence
  void push_front(tuple_type const& t)
  {
    SYSX_ASSERT(!t.is_infinite());

    buf_.push_front(t);
    add_duration(t.duration());
  }

  /// remove front of the sequence
  void pop_front()
  {
    SYSX_ASSERT(!empty());
    del_duration(front().duration());
    buf_.pop_front();
  }

  /// remove front of the sequence for a given duration
  duration_type pop_front(duration_type d)
  {
    SYSX_ASSERT(!empty());

    if (d == duration_type::zero_time) {
      buf_.pop_front();
      return d;
    }

    iterator it = buf_.begin();
    while (it != buf_.end() && d >= it->duration()) {
      if (d == duration_type::zero_time)
        break;
      d -= it->duration();
      del_duration(it->duration());
      ++it;
    }
    if (it != buf_.begin()) // drop fully covered tuples
      buf_.erase(buf_.begin(), it);
    return d;
  }

  ///\}

  // ---------------------------------------------------------------------
  /** \name access tail of the sequence */
  ///\{

  /// read front of the sequence
  const_reference back() const { return buf_.back(); }
  reference back() { return buf_.back(); }

  /// update/replace (value of) last element in the sequence
  void back(value_type const& v) { buf_.back().value(v); }

  /// update/replace last element in the sequence
  void back(value_type const& v, duration_type const& d)
  {
    back(tuple_type(v, d));
  }

  /// update/replace last element in the sequence
  void back(tuple_type const& t)
  {
    // SYSX_ASSERT(!t.is_infinite());
    duration_type d = buf_.back().duration();
    buf_.back() = t;
    if (t.duration() < d) { // shorter tuple
      del_duration(d - t.duration());
    } else {
      add_duration(t.duration() - d);
    }
  }

  /// remove tail of the sequence
  void pop_back()
  {
    SYSX_ASSERT(!empty());
    const_reference last = back();
    if (!last.is_infinite()) {
      del_duration(last.duration());
    } else { // need to recompute duration
      set_duration(duration_type());
      for (const_iterator it = buf_.begin(); it != buf_.end() - 1; ++it)
        add_duration(it->duration());
    }
    buf_.pop_back();
  }
  ///\}

  /// split at a given offset
  void split(duration_type const& offset);

  // ---------------------------------------------------------------------
  /** \name sub-range interface */
  ///\{

  range_type before(duration_type const& until)
  {
    return range_type(*this, duration_type(), until, false);
  }
  const_range_type before(duration_type const& until) const
  {
    return const_cast<this_type*>(this)->before(until);
  }
  const_range_type cbefore(duration_type const& until) const
  {
    return before(until);
  }

  range_type range(duration_type const& until)
  {
    return range_type(*this, duration_type(), until, true);
  }
  const_range_type range(duration_type const& until) const
  {
    return const_cast<this_type*>(this)->range(until);
  }
  const_range_type crange(duration_type const& until) const
  {
    return range(until);
  }

  range_type range(duration_type const& from, duration_type const& to)
  {
    return range_type(*this, from, to, true);
  }
  const_range_type range(duration_type const& from,
                         duration_type const& to) const
  {
    return const_cast<this_type*>(this)->range(from, to);
  }
  const_range_type crange(duration_type const& from,
                          duration_type const& to) const
  {
    return range(from, to);
  }

  ///\}

  /** \name const tuple iterators */
  ///\{
  const_iterator begin() const { return buf_.begin(); }
  const_iterator end() const { return buf_.end(); }
  const_iterator cbegin() const { return buf_.begin(); }
  const_iterator cend() const { return buf_.end(); }
  ///\}

  /** \name local time interface */
  ///\{
  duration_type front_duration() const { return front().duration(); }
  ///\}

  // ---------------------------------------------------------------------
  void print(std::ostream& os = std::cout) const;

  friend bool operator==(this_type const& lhs, this_type const& rhs)
  {
    return lhs.duration() == rhs.duration() && lhs.buf_ == rhs.buf_;
  }

  friend std::ostream& operator<<(std::ostream& os, this_type const& t)
  {
    t.print(os);
    return os;
  }

protected:
  using base_type::add_duration;
  using base_type::del_duration;
  using base_type::duration_;

  storage_type buf_;
}; // timed_sequence

// -----------------------------------------------------------------------

} // namespace tracing

#include "tvs/tracing/timed_sequence.tpp"

#endif /* TVS_TIMED_SEQUENCE_H_INCLUDED_ */
/* Taf!
 */
