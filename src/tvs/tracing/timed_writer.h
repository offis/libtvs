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
 * \file   timed_writer.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed value writer for tracing (implementation)
 * \see    timed_stream.h
 */

#ifndef TVS_TIMED_WRITER_H_INCLUDED_
#define TVS_TIMED_WRITER_H_INCLUDED_

#include "tvs/tracing/timed_sequence.h"
#include "tvs/tracing/timed_writer_base.h"

#include "tvs/tracing/timed_variant.h"

namespace tracing {

// forward declarations
template<typename, typename>
class timed_stream;

template<typename>
struct timed_state_traits;

template<typename T, typename Traits = timed_state_traits<T>>
class timed_writer : public timed_writer_base
{
  friend class timed_stream<T, Traits>;

public:
  typedef timed_writer_base base_type;
  typedef timed_stream<T, Traits> stream_type;
  typedef T value_type;
  typedef timed_value<T> tuple_type;

  explicit timed_writer(stream_type& stream)
    : base_type()
    , stream_(&stream)
  {
    base_type::attach(stream);
  }

  explicit timed_writer(const char* nm, writer_mode mode = STREAM_DEFAULT)
    : base_type(mode & STREAM_CREATE ? create_stream(nm, mode) : NULL)
    , stream_()
  {
    base_type::attach(nm);
    stream_ = static_cast<stream_type*>(&base_type::stream());
  }

  //! push interface
  //!{
  void push(value_type const& v, duration_type const& dur)
  {
    this->push(tuple_type(v, dur));
  }

  void push(time_type const& offset,
            value_type const& value,
            duration_type const& dur)
  {
    this->push(offset, tuple_type(value, dur));
  }

  void push(value_type const& value) { stream_->push(value); }
  void push(tuple_type const& tuple) { stream_->push(tuple); }

  void push(time_type const& offset, tuple_type const& tuple)
  {
    stream_->push(offset, tuple);
  }

  void push_variant(timed_variant const& var) override
  {
    auto const& val = var.value().get<value_type>();
    this->push(val, var.duration());
  }
  //!}

protected:
  static stream_type* create_stream(const char* nm, writer_mode mode);

private:
  stream_type* stream_;
};

/* ----------------------------- constructor --------------------------- */

template<typename T, typename P>
typename timed_writer<T, P>::stream_type*
timed_writer<T, P>::create_stream(const char* name, writer_mode mode)
{
  // prefer to attach to existing stream
  if (mode & STREAM_ATTACH && // STREAM_AUTO (lazy create)
      lookup(name))
    return NULL;

  return new stream_type(name);
}

} // namespace tracing

#endif /* TVS_TIMED_WRITER_H_INCLUDED_ */
/* Taf!
 */
