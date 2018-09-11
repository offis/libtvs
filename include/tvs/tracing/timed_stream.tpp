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
 * \file   timed_stream.tpp
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed value stream (template implementation)
 * \see    timed_stream.h
 */

#include <tvs/tracing/timed_reader.h>
#include <tvs/tracing/timed_stream.h>
#include <tvs/tracing/timed_writer.h>

#include <tvs/tracing/timed_stream_traits.h>

#include <tvs/utils/unique_ptr.h>

#include <tvs/utils/debug.h>
#include <tvs/utils/macros.h>

namespace tracing {

template<typename T, typename Traits>
void
timed_stream<T, Traits>::merge_future(sequence_type&& other)
{
  sequence_type& this_ = this->future_;

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

    // special case: handle 'split' at 0
    if (seq_b->front_duration() == duration_type::zero_time) {
      tuple_type zero_front = seq_a->front();
      zero_front.duration(duration_type::zero_time);
      seq_a->push_front(zero_front);
    } else {
      // split the front tuple of A
      seq_a->split(seq_b->front_duration());
    }

    tuple_type a_front = seq_a->front();
    merge_policy::merge(a_front, seq_b->front());

    result.push_back(a_front);
    seq_b->pop_front();
    seq_a->pop_front();
  }

  // handle leftover tuples in both sequences
  if (!seq_a->empty())
    result.move_back(*seq_a);
  if (!seq_b->empty())
    result.move_back(*seq_b);

  this_.clear();
  this_.move_back(result);
}

/* -------------------------- push interface -------------------------- */

template<typename T, typename P>
void
timed_stream<T, P>::push(tuple_type const& t)
{
  if (future_.empty()) {
    buf_.push_back(t);
  } else {

    // consume any future values caused by the local offset increment
    sequence_type tmp;
    tmp.push_back(t);
    merge_future(std::move(tmp));

    auto merge_range = future_.range(t.duration());

    SYSX_ASSERT(merge_range.duration() == t.duration());

    buf_.push_back(merge_range.begin(), merge_range.end());
    future_.pop_front(merge_range.duration());
  }
}

template<typename T, typename P>
void
timed_stream<T, P>::push(value_type const& val)
{
  tuple_type tup(val, duration_type::infinity());

  if (future_.front().is_infinite()) {
    future_.front(tup);
  } else {
    // merge with existing future sequence
    sequence_type pushed;
    pushed.push_back(tup);
    merge_future(std::move(pushed));
  }
}

template<typename T, typename P>
void
timed_stream<T, P>::push(time_type offset, tuple_type const& tuple)
{
  sequence_type pushed;

  if (offset > duration_type::zero_time)
    pushed.push_back(empty_policy::empty(offset));

  pushed.push_back(tuple);
  merge_future(std::move(pushed));
}

/* ------------------------- commit interface ------------------------- */

template<typename T, typename P>
void
timed_stream<T, P>::do_pre_commit_reader(duration_type const& dur)
{

  // do we need to prepare anything at all?
  if (dur == duration())
    return;

  // check if we can just split inside existing buffer
  if (dur < duration()) {
    buf_.split(dur);
    return;
  }

  // determine offset into future sequence
  duration_type fdur = dur - duration();

  // extend if necessary
  if (fdur > future_.duration())
    future_.push_back(empty_policy::empty(fdur - future_.duration()));

  future_.split(fdur);

  // append from future so we can satisfy the commit
  auto range = future_.range(fdur);

  buf_.push_back(range.begin(), range.end());

  future_.pop_front(fdur);
}

template<typename T, typename P>
void
timed_stream<T, P>::do_commit_reader(timed_reader_base& r,
                                     duration_type const& dur,
                                     bool last)
{
  typedef timed_reader<T, P> reader_type;
  reader_type& reader = static_cast<reader_type&>(r);

  if (dur == duration_type::zero_time)
    return;

  bool new_window = reader.buf_.empty();

  if (dur == duration()) {
    if (!last) {
      reader.buf_.push_back(buf_);
    } else {
      reader.buf_.move_back(buf_);
    }
  } else {
    // partially commit, buf_ has already been prepared
    auto range = buf_.range(dur);
    reader.buf_.push_back(range.begin(), range.end());

    if (last)
      buf_.pop_front(dur);
  }

  reader.trigger(new_window);
}

template<typename T, typename Traits>
void
timed_stream<T, Traits>::print(std::ostream& os) const
{
  os << "@" << this->local_time() << " : " << buf_ << "|" << future_ << "\n";
}

} // namespace tracing

/* Taf!
 */
