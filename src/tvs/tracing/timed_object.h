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
 * \file   timed_object.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed object base class
 * \see    timed_stream.h timed_value.h
 */

#ifndef TVS_TIMED_OBJECT_H_INCLUDED_
#define TVS_TIMED_OBJECT_H_INCLUDED_

#include "tvs/tracing/timed_duration.h" // time_type
#include "tvs/utils/noncopyable.h"

#include <functional>

namespace tracing {

class timed_object;
class timed_stream_base;

namespace host {

using cb_type = std::function<bool(timed_stream_base*)>;
using sync_fn_type = std::function<void(time_type const&)>;

void for_each_stream_in_scope(cb_type);

/// Synchronise with the simulation/implementation model time and the
/// configured sync function.
void
sync_with_model(time_type until);

const char*
gen_unique_name(const char* name);

timed_stream_base*
lookup(const char* name);

} // namespace host

/// Register a synchronisation function to be called for synchronising the time
/// with a simulation model.  This defaults to sc_core::wait(time_type) iff
/// SystemC support is available.
void
register_sync(host::sync_fn_type fn);

/// Perform a commit until the maximum local time offset of all streams in the
/// scope.
///
/// \returns the absolute time which all streams in the scope have been advanced to.
time_type
sync();

/// Perform a commit on all streams in the scope with the given absolute time.
void
sync(time_type const& until);


#define TVS_SYNCHRONISATON_POINT()                                             \
  ::tracing::host::sync_with_model(::tracing::sync())

class timed_base
{
public:
  typedef tracing::time_type time_type;
  typedef tracing::timed_duration duration_type;

  time_type const& local_time() const { return time_; }

  void commit();
  void commit(time_type const& until);
  void commit(duration_type const& duration);

  /// Perform a commit on all streams in the scope with the given duration,
  /// relative to this stream's local time
  time_type sync(duration_type const& duration)
  {
    time_type t = this->local_time() + duration;
    ::tracing::sync(t);
    return t;
  }

protected:
  timed_base();
  timed_base(const time_type& now);

  virtual duration_type do_commit(duration_type until);

  /// protected destructor - no polymorphic destruction
  virtual ~timed_base() /* = default */ {}

private:
  time_type time_;
};

#ifndef SYSX_NO_SYSTEMC
typedef sc_core::sc_object named_object;
#else
class named_object
{
public:
  virtual const char* name() const;
  virtual const char* kind() const;

  virtual const char* basename() const { return name(); }

protected:
  named_object(const char* nm);

  /// protected destructor - no polymorphic destruction
  virtual ~named_object();

private:
  std::string name_;
};
#endif // SYSX_NO_SYSTEMC

class timed_object : public named_object, public timed_base
{
public:
  const char* kind() const override { return "timed_object"; }

protected:
  explicit timed_object(const char* nm)
    : named_object(nm)
  {
  }

  /// protected destructor - no polymorphic destruction
  ~timed_object() override /* = default */ {}
};

} // namespace tracing

#endif /* TVS_TIMED_OBJECT_H_INCLUDED_ */
/* Taf!
 */
