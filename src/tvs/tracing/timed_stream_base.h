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
 * \file   timed_stream_base.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed value streams for tracing (base class)
 * \see    timed_stream.h timed_value.h
 */

#ifndef TVS_TIMED_STREAM_BASE_H_INCLUDED_
#define TVS_TIMED_STREAM_BASE_H_INCLUDED_

#include "tvs/tracing/timed_duration.h"
#include "tvs/tracing/timed_object.h"

namespace tracing {

// forward declarations
class timed_reader_base;
class timed_writer_base;

/// type-independent base class for timed streams
class timed_stream_base : public timed_object
{
public:
  typedef sc_core::sc_time time_type;
  typedef timed_duration duration_type;

  virtual ~timed_stream_base();

  void attach(timed_writer_base& writer);
  void attach(timed_reader_base& reader);

  void detach(timed_writer_base& writer);
  void detach(timed_reader_base& reader);

  time_type begin_time() const { return local_time(); }
  time_type end_time() const { return local_time() + duration(); }
  virtual duration_type duration() const = 0;

protected:
  explicit timed_stream_base(const char* nm = default_name());

  duration_type do_commit(duration_type until);

  static const char* default_name();

private:
  virtual bool do_type_check(timed_reader_base const& r) const = 0;
  virtual bool do_type_check(timed_writer_base const& w) const = 0;

  virtual void do_pre_commit_reader(duration_type const&) = 0;
  virtual void do_commit_reader(timed_reader_base& r,
                                duration_type const& until,
                                bool last = false) = 0;
  virtual void do_clear() = 0;

  timed_writer_base* writer_;
  std::vector<timed_reader_base*> readers_;
}; // class timed_value_base

} // namespace tracing

#endif /* TVS_TIMED_STREAM_BASE_H_INCLUDED_ */
/* Taf!
 */
