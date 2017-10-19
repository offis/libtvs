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
 * \file   timed_reader_base.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed value streams for tracing (reader base interface)
 * \see    timed_stream.h
 */

#ifndef TVS_TIMED_READER_BASE_H_INCLUDED_
#define TVS_TIMED_READER_BASE_H_INCLUDED_

#include "tvs/tracing/timed_object.h"
#include "tvs/tracing/timed_value.h"
#include "tvs/tracing/timed_variant.h"

namespace tracing {

// forward declarations
class timed_stream_base;
class timed_reader_base;

class timed_listener_if
{
  friend class timed_reader_base;

public:
  virtual void notify(timed_reader_base& s) = 0;

protected:
  typedef unsigned listener_mode;

  enum
  {
    NOTIFY_NONE = 0x0,
    NOTIFY_WINDOW = 0x1,
    NOTIFY_APPEND = 0x2,
    NOTIFY_COMMIT = NOTIFY_WINDOW | NOTIFY_APPEND,
    NOTIFY_DEFAULT = NOTIFY_COMMIT
  };

  virtual ~timed_listener_if() {}

}; // class timed_listener_if

class timed_reader_base : public timed_object
{
  friend class timed_stream_base;
  typedef timed_stream_base stream_type;

public:
  typedef std::size_t size_type;
  typedef timed_listener_if::listener_mode listener_mode;

  /** \name distributed time interface */
  ///\{
  time_type next_time() const { return local_time() + front_duration(); }
  ///\}

  /** \name query available tuples */
  ///\{
  bool empty() const;
  virtual size_type count() const = 0; ///< number of available tuples

  /// are tuples available?
  bool available() const;
  /// absolute time until tuples are available
  time_type available_until() const;
  /// duration for which tuples are available
  virtual duration_type available_duration() const = 0;
  ///\}

  /** \name consume available tuples */
  ///\{
  void pop();
  void pop_all();
  void pop_until(time_type const&);
  void pop_duration(duration_type const&);
  ///\}

  /** \name access stream front */
  ///\{
  virtual timed_value_base const& front() const = 0;
  virtual timed_value_base const& front(duration_type const& offset) = 0;
  virtual timed_variant front_variant() const = 0;
  virtual timed_variant front_variant(duration_type const& offset) = 0;

  duration_type front_duration() const { return front().duration(); }
  ///\}

  virtual ~timed_reader_base();

  void attach(const char* name);
  void attach(stream_type& stream);
  void detach();

  listener_mode listen(timed_listener_if& listener,
                       listener_mode mode = timed_listener_if::NOTIFY_DEFAULT);

  virtual stream_type& stream() { return *stream_; }
  virtual stream_type const& stream() const { return *stream_; }

protected:
  timed_reader_base(const char* name);

  virtual void do_pop_duration(duration_type const&) = 0;
  void trigger(bool new_window);

private:
  stream_type* stream_;
  timed_listener_if* listener_;
  listener_mode listen_mode_;
};

inline bool
timed_reader_base::empty() const
{
  return (count() == 0);
}

inline bool
timed_reader_base::available() const
{
  return !empty();
}

inline time_type
timed_reader_base::available_until() const
{
  return local_time() + available_duration();
}

inline void
timed_reader_base::trigger(bool new_window)
{
  if (listen_mode_ & (new_window ? timed_listener_if::NOTIFY_WINDOW
                                 : timed_listener_if::NOTIFY_APPEND))
    listener_->notify(*this);
}

inline void
timed_reader_base::pop()
{
  pop_duration(front_duration());
}

inline void
timed_reader_base::pop_all()
{
  pop_duration(available_duration());
}

inline void
timed_reader_base::pop_until(time_type const& time)
{
  SYSX_ASSERT(time > local_time() && time <= available_until());
  do_pop_duration(time - local_time());
}

inline void
timed_reader_base::pop_duration(duration_type const& dur)
{
  SYSX_ASSERT(dur <= available_duration());
  do_pop_duration(dur);
}

} // namespace tracing

#endif /* TVS_TIMED_READER_BASE_H_INCLUDED_ */
/* Taf!
 */
