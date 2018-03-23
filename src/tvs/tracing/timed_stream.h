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
 * \file   timed_stream.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed value streams for tracing
 * \see    timed_tuple.h
 */

#ifndef TVS_TIMED_STREAM_H_INCLUDED_
#define TVS_TIMED_STREAM_H_INCLUDED_

#include "tvs/tracing/timed_sequence.h"
#include "tvs/tracing/timed_stream_base.h"
#include "tvs/tracing/timed_value.h"

#include "report_msgs.h"

namespace tracing {

template<typename, typename>
class timed_writer;
template<typename, typename>
class timed_reader;

template<typename>
struct timed_state_traits;

// timed_stream< int >  -> timed_stream< int, timed_stream_policies< int > >

template<typename T, typename Traits = timed_state_traits<T>>
class timed_stream
  : public timed_stream_base
  , protected Traits::empty_policy
  , protected Traits::split_policy
  , protected Traits::merge_policy
{
  friend class timed_writer<T, Traits>;
  friend class timed_reader<T, Traits>;

public:
  typedef timed_stream_base base_type;
  typedef timed_stream<T, Traits> this_type;
  typedef T value_type;
  typedef timed_value<T> tuple_type;

  typedef timed_reader<T, Traits> reader_type;
  typedef timed_writer<T, Traits> writer_type;

  typedef typename Traits::empty_policy empty_policy;
  typedef typename Traits::split_policy split_policy;
  typedef typename Traits::merge_policy merge_policy;

  typedef timed_sequence<T, Traits> sequence_type;

  explicit timed_stream(const char* nm = "timed_stream")
    : base_type(nm)
  {}

  duration_type duration() const override { return buf_.duration(); }

  void print(std::ostream& os = std::cout) const override;

  friend std::ostream& operator<<(std::ostream& os, this_type const& t)
  {
    t.print(os);
    return os;
  }

protected:
  /** \name push interface */
  ///\{

  void push(tuple_type const&);

  void push(value_type const&);

  void push(time_type offset, tuple_type const&);

  ///\}

protected:
  bool do_type_check(timed_reader_base const& r) const override
  {
    return (dynamic_cast<reader_type const*>(&r) != NULL);
  }

  bool do_type_check(timed_writer_base const& w) const override
  {
    return (dynamic_cast<writer_type const*>(&w) != NULL);
  }

  void do_pre_commit_reader(duration_type const&) override;
  void do_commit_reader(timed_reader_base& r,
                                duration_type const& until,
                                bool last = false) override;
  void do_clear() override { buf_.clear(); }

private:
  void merge_future(sequence_type&& other);

  sequence_type buf_;
  sequence_type future_;
};

// retrieve a timed_stream<T, Traits> by its hieractical name
template<typename StreamType>
StreamType&
stream_by_name(const char* stream)
{
  using stream_type = StreamType;
  auto str = dynamic_cast<stream_type*>(host::lookup(stream));

  if (str == nullptr) {
    SYSX_REPORT_ERROR(report::stream_lookup) % stream
      << "stream type mismatch";
  }

  return *str;
}

} // namespace tracing

#include "timed_stream.tpp"

#endif /* TVS_TIMED_STREAM_H_INCLUDED_ */
/* Taf!
 */
