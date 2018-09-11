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
 * \file   timed_ranges.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  references to partial sequences of timed values
 * \see    timed_value.h, timed_sequence.h
 *
 * This is an internal class used by the \ref timed_stream implementation
 * to hold a reference to an intermediate partial \ref timed_sequence of
 * \ref timed_value instances.
 */

#ifndef TVS_TIMED_RANGES_H_INCLUDED_
#define TVS_TIMED_RANGES_H_INCLUDED_

#include <tvs/tracing/timed_sequence.h>

namespace tracing {

/// non-mutating reference to a (sub)range of a timed_sequence
template<typename T, typename Traits>
class const_timed_range
{
  friend class timed_sequence<T, Traits>;

public:
  typedef const_timed_range this_type;
  typedef timed_sequence<T, Traits> sequence_type;
  typedef typename sequence_type::duration_type duration_type;
  typedef typename sequence_type::value_type value_type;
  typedef typename sequence_type::tuple_type tuple_type;
  typedef typename sequence_type::iterator iterator;
  typedef typename sequence_type::const_iterator const_iterator;
  typedef typename sequence_type::const_reference const_reference;
  typedef typename sequence_type::size_type size_type;

  bool empty() const { return size() == 0; }
  time_type offset() const { return offset_; }
  duration_type duration() const { return duration_; }
  size_type size() const { return end_ - begin_; }

  /** \name const tuple iterators */
  ///\{
  const_iterator begin() const { return begin_; }
  const_iterator end() const { return end_; }
  const_iterator cbegin() const { return begin_; }
  const_iterator cend() const { return end_; }
  ///\}

  /// read front of the range
  const_reference front() const { return *begin(); }
  const_reference back() const { return *(end() - 1); }

  void print(std::ostream& os = std::cout) const;
  friend std::ostream& operator<<(std::ostream& os, this_type const& t)
  {
    t.print(os);
    return os;
  }

protected:
  const_timed_range(sequence_type& owner,
                    time_type offset,
                    time_type until,
                    bool covering);

protected:
  sequence_type& ref_;
  duration_type offset_;
  duration_type duration_;
  iterator begin_;
  iterator end_;
}; // timed_range

/// mutating reference to a (sub)range of a timed_sequence
template<typename T, typename Traits>
class timed_range : public const_timed_range<T, Traits>
{
  friend class timed_sequence<T, Traits>;

public:
  typedef const_timed_range<T, Traits> base_type;
  typedef timed_range<T, Traits> this_type;

  typedef typename base_type::sequence_type sequence_type;
  typedef typename base_type::duration_type duration_type;
  typedef typename base_type::value_type value_type;
  typedef typename base_type::tuple_type tuple_type;
  typedef typename sequence_type::reference reference;
  typedef typename sequence_type::iterator iterator;

  /// read front of the range
  reference front() const { return *this->begin_; }
  reference back() const { return *(this->end_ - 1); }

  /// update/replace (value of) first element in the range
  void front(value_type const& v) { this->ref_.buf_.front().value(v); }

  /// update/replace first element in the range
  void front(value_type const& v, duration_type const& d)
  {
    front(tuple_type(v, d));
  }

  /// update/replace first element in the range
  void front(tuple_type const& t)
  {
    duration_type d = this->front().duration();
    SYSX_ASSERT(!(d.is_infinite() ^ t.is_infinite()));
    *this->begin_ = t;
    update_range_duration(t, d);
  }

  /// update/replace (value of) first element in the range
  void back(value_type const& v) { (this->end_ - 1)->value(v); }

  /// update/replace first element in the range
  void back(value_type const& v, duration_type const& d)
  {
    back(tuple_type(v, d));
  }

  /// update/replace last element in the range
  void back(tuple_type const& t)
  {
    duration_type d = this->back().duration();
    SYSX_ASSERT(!(d.is_infinite() ^ t.is_infinite()));
    *(this->end_ - 1) = t;
    update_range_duration(t, d);
  }

  /// replace the sub-sequence with another sequence (of the same duration)
  template<typename SequenceType>
  void replace(SequenceType const& seq)
  {
    SYSX_ASSERT(this->duration_ == seq.duration());
    iterator pos = this->ref_.buf_.erase(this->begin_, this->end_);
    this->begin_ = this->ref_.buf_.insert(pos, seq.begin(), seq.end());
    this->end_ = this->begin_ + seq.size();
  }

protected:
  timed_range(sequence_type& owner,
              time_type offset,
              time_type until,
              bool covering)
    : base_type(owner, offset, until, covering)
  {}

  void update_range_duration(tuple_type t, duration_type d)
  {
    if (t.duration() < d) { // shorter tuple
      this->ref_.del_duration(d - t.duration());
      this->duration_ -= d - t.duration();
    } else if (t.duration() > d) { // longer tuple
      this->ref_.add_duration(t.duration() - d);
      this->duration_ += t.duration() - d;
    }
  }

}; // timed_range

// -----------------------------------------------------------------------

template<typename T, typename Traits>
const_timed_range<T, Traits>::const_timed_range(sequence_type& owner,
                                                time_type offset,
                                                time_type until,
                                                bool covering)
  : ref_(owner)
  , offset_()
  , duration_()
  , begin_(owner.buf_.begin())
  , end_(owner.buf_.end())
{
  SYSX_ASSERT(until >= offset);
  if (offset >= owner.duration()) {
    begin_ = end_;
    offset_ = offset;
    return;
  }

  // determine offset
  for (; begin_ != end_ && offset_ < offset; ++begin_) {
    duration_type d = begin_->duration();
    if (covering && offset_ + d > offset)
      break;
    offset_ += d;
  }
  // empty range
  if (begin_ == end_)
    return;

  // determine duration
  for (end_ = begin_; end_ != owner.end() && duration_ < until - offset_;
       ++end_) {
    duration_type d = end_->duration();
    if (!covering && duration_ + d > until - offset_)
      break;
    duration_ += d;
  }

  // catch all zero-time events at the edge of the range
  if (duration_ == until - offset_) {
    while (end_ != owner.end() && end_->duration().is_delta())
      end_++;
  }
}

template<typename T, typename Traits>
void
const_timed_range<T, Traits>::print(std::ostream& os) const
{
  os << "{" << offset_ << ", " << this->duration() << "; ";
  if (begin_ == end_) {
    os << "- }";
    return;
  }
  for (auto elem : *this)
    os << elem;

  os << " }";
}

// -----------------------------------------------------------------------

} // namespace tracing

#endif /* TVS_TIMED_RANGES_H_INCLUDED_ */
/* Taf!
 */
