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
 * \file   timed_reader.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed value streams for tracing (reader interface)
 * \see    timed_value.h, timed_stream.h
 */

#ifndef TVS_TIMED_READER_H_INCLUDED_
#define TVS_TIMED_READER_H_INCLUDED_

#include <tvs/tracing/timed_reader_base.h>
#include <tvs/tracing/timed_sequence.h>
#include <tvs/tracing/timed_variant.h>

namespace tracing {

template<typename, typename>
class timed_stream;

template<typename>
struct timed_state_traits;

template<typename T, typename Traits = timed_state_traits<T>>
class timed_reader : public timed_reader_base
{
  friend class timed_stream<T, Traits>;

public:
  typedef timed_reader_base base_type;
  typedef base_type::time_type time_type;
  typedef timed_reader<T, Traits> this_type;

  typedef T value_type;
  typedef timed_stream<T, Traits> stream_type;
  typedef timed_value<T> tuple_type;

  typedef typename stream_type::sequence_type sequence_type;

  typedef typename sequence_type::const_iterator const_iterator;
  typedef typename sequence_type::range_type range_type;
  typedef typename sequence_type::const_range_type const_range_type;

  /** \name constructors */
  ///\{
  timed_reader(const char* name)
    : timed_reader_base(name)
  {}

  timed_reader(const char* name, stream_type& stream)
    : timed_reader_base(name)
  {
    base_type::attach(stream);
  }

  timed_reader(const char* name, const char* stream_name)
    : timed_reader_base(name)
  {
    base_type::attach(stream_name);
  }

  timed_reader(timed_reader&&) = delete;
  timed_reader& operator=(timed_reader&&) = delete;

  timed_reader(timed_reader const&) = delete;
  timed_reader& operator=(timed_reader const&) = delete;

  ///\}

  // read a value at a given time (between start and end time)
  value_type const& get() const { return front().value(); }
  // allow modifying the value of the front tuple
  value_type& get() { return buf_.front().value(); }
  value_type const& get(time_type const& offset) const;
  value_type const& get(duration_type const& offset) const;

  // read (and potentially split) the first tuple
  tuple_type const& front() const override { return buf_.front(); }
  tuple_type const& front(duration_type const& dur) override
  {
    if (front_duration() > dur) {
      // special case: splitting at zero-time should produce a new zero-time
      // tuple at the front
      if (dur == duration_type::zero_time) {
        buf_.push_front(get(), dur);
      } else {
        buf_.split(dur);
      }
    }

    SYSX_ASSERT(front_duration() <= dur);

    return front();
  }

  timed_variant front_variant() const override
  {
    return timed_variant(buf_.front().value(), front_duration());
  }

  timed_variant front_variant(duration_type const& dur) override
  {
    front(dur);

    return front_variant();
  }

  /** \name const tuple iterators */
  ///\{
  const_iterator begin() const { return buf_.begin(); }
  const_iterator end() const { return buf_.end(); }
  const_iterator cbegin() const { return buf_.begin(); }
  const_iterator cend() const { return buf_.end(); }
  ///\}

  // ---------------------------------------------------------------------
  /** \name sub-range interface */
  ///\{
  range_type before(duration_type const& until)
  {
    buf_.split(until);
    return buf_.before(until);
  }

  const_range_type before(duration_type const& until) const
  {
    buf_.split(until);
    return buf_.before(until);
  }
  const_range_type cbefore(duration_type const& until) const
  {
    return before(until);
  }

  range_type range(duration_type const& until)
  {
    buf_.split(until);
    return buf_.range(until);
  }
  const_range_type range(duration_type const& until) const
  {
    buf_.split(until);
    return buf_.range(until);
  }
  const_range_type crange(duration_type const& until) const
  {
    return range(until);
  }

  range_type range(duration_type const& from, duration_type const& to)
  {
    buf_.split(from);
    buf_.split(to);
    return buf_.range(from, to);
  }
  const_range_type range(duration_type const& from,
                         duration_type const& to) const
  {
    buf_.split(from);
    buf_.split(to);
    return buf_.range(from, to);
  }
  const_range_type crange(duration_type const& from,
                          duration_type const& to) const
  {
    return range(from, to);
  }
  ///\}

  size_type count() const override { return buf_.size(); }
  duration_type available_duration() const override { return buf_.duration(); }

  void print(std::ostream& os = std::cout) const override
  {
    os << name() << "@" << local_time() << ": " << buf_;
  }

private:
  void do_pop_duration(duration_type const& d) override
  {
    buf_.split(d);
    duration_type rem = buf_.pop_front(d);

    SYSX_ASSERT(rem == duration_type::zero_time || buf_.empty());
    commit(d);
  }

  sequence_type buf_;
};

} // namespace tracing

#endif /* TVS_TIMED_READER_H_INCLUDED_ */
/* Taf!
 */
