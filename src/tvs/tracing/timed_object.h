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

#include "tvs/tracing/object_host.h"
#include "tvs/tracing/timed_duration.h" // time_type
#include "tvs/utils/noncopyable.h"

namespace tracing {

#define TVS_SYNCHRONISATON_POINT()                                             \
  ::tracing::object_host::sync_with_model(                                     \
    ::tracing::timed_base::sync_current_scope())

class timed_base : protected object_host
{
public:
  typedef tracing::time_type time_type;
  typedef tracing::timed_duration duration_type;

  time_type const& local_time() const { return time_; }

  void commit();
  void commit(time_type const& until);
  void commit(duration_type const& duration);

  /// Perform a commit until the maximum local time offset of all streams in the
  /// scope.
  time_type sync();

  /// Perform a commit on all streams in the scope with the given absolute time.
  time_type sync(time_type const& until);

  /// Perform a commit on all streams in the scope with the given duration.
  time_type sync(duration_type const& duration);

  /// Call sync() on the first stream found in the scope
  static time_type sync_current_scope();

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
  virtual ~named_object() /* = default */ {}

private:
  std::string name_;
};
#endif // SYSX_NO_SYSTEMC

class timed_object
  : public named_object
  , public timed_base
{
public:
  const char* kind() const { return "timed_object"; }

protected:
  explicit timed_object(const char* nm)
    : named_object(nm)
  {}

  /// protected destructor - no polymorphic destruction
  virtual ~timed_object() /* = default */ {}
};

} // namespace tracing

#endif /* TVS_TIMED_OBJECT_H_INCLUDED_ */
/* Taf!
 */
