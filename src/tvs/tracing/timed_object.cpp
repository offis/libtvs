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
 * \file   timed_object.cpp
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed streaming base classes
 * \see    timed_object.h
 */

#include "tvs/tracing/timed_object.h"
#include "tvs/tracing/timed_stream_base.h"

#include "report_msgs.h"

#include "tvs/utils/debug.h"
#include "tvs/utils/macros.h"

#ifdef SYSX_NO_SYSTEMC
#include <map>
#endif

namespace {

tracing::host::sync_fn_type sync_fn;

#ifdef SYSX_NO_SYSTEMC

/// object registry for timed_object instances
std::map<std::string, tracing::named_object*> object_registry;

#endif // SYSX_NO_SYSTEMC

} // anonymous namespace

namespace tracing {

void
register_sync(host::sync_fn_type fn)
{
  if (sync_fn) {
    SYSX_REPORT_WARNING(sysx::report::plain_msg)
      << "Overriding already defined synchronisation function.";
  }
  sync_fn = fn;
}

namespace host {

void
sync_with_model(time_type until)
{
  if (!sync_fn) {
#ifndef SYSX_NO_SYSTEMC
    SYSX_REPORT_WARNING(sysx::report::plain_msg)
      << "Setting sc_core::wait as the default synchronisation function.";
    sync_fn = [](time_type const& until) { ::sc_core::wait(until); };
#else
    SYSX_REPORT_FATAL(sysx::report::plain_msg)
      << "Cannot synchronise: no sync method specified."
      << "Please provide a sync callback with "
         "::tracing::register_sync().";
#endif
  }
  sync_fn(until);
}

/// Apply func on all streams in the current SystemC module scope.
///
/// The iteration stops when func returns true.
void
for_each_stream_in_scope(host::cb_type func)
{
#ifdef SYSX_NO_SYSTEMC
  SYSX_REPORT_FATAL(sysx::report::not_implemented) % "native scope traversal";
#else
  sc_core::sc_object* scope = sc_core::sc_get_current_object();
  SYSX_ASSERT(scope != nullptr);

  scope = scope->get_parent_object();
  SYSX_ASSERT(scope != nullptr);

  for (auto& obj : scope->get_child_objects()) {
    auto* stream = dynamic_cast<timed_stream_base*>(&(*obj));
    if (stream != nullptr) {
      if (func(stream))
        break;
    }
  }
#endif
}

const char*
gen_unique_name(const char* name)
{
#ifdef SYSX_NO_SYSTEMC
  static std::vector<std::string> names_;
  static int num = 0;

  auto it = object_registry.find(name);

  if (it != object_registry.end()) {
    std::stringstream sstr;
    sstr << name << "_" << num++;
    names_.emplace_back(sstr.str());
    return names_.back().c_str();
  }

  return name;

#else
  return sc_core::sc_gen_unique_name(name);
#endif // SYSX_NO_SYSTEMC
}

tracing::timed_stream_base*
lookup(const char* name)
{
#ifdef SYSX_NO_SYSTEMC
  auto it = object_registry.find(name);

  if (it == object_registry.end()) {
    SYSX_REPORT_ERROR(report::stream_lookup) % name << "object not found";
    return nullptr;
  }

  return dynamic_cast<timed_stream_base*>(it->second);
#else
  auto str = dynamic_cast<timed_stream_base*>(sc_core::sc_find_object(name));

  if (!str) {
    auto scope = sc_core::sc_get_current_object();
    if (scope) {
      std::stringstream lname;
      lname << scope->name() << sc_core::SC_HIERARCHY_CHAR << name;
      str = host::lookup(lname.str().c_str());
    }

    if (!str) {
      SYSX_REPORT_ERROR(report::stream_lookup) % name
        << "object not found "
        << "(scope: " << (scope ? scope->name() : "<top>") << ")";
      return nullptr;
    }
  }

  return str;
#endif // SYSX_NO_SYSTEMC
}

} // namespace host

/* ----------------------------- sync --------------------------- */

time_type
sync()
{
  time_type max_time;
  host::for_each_stream_in_scope([&max_time](timed_stream_base* stream) {
    max_time = std::max(max_time, stream->end_time());
    return false;
  });

  sync(max_time);

  return max_time;
}

void
sync(time_type const& until)
{
  host::for_each_stream_in_scope([&until](timed_stream_base* stream) {
    stream->commit(until);
    return false;
  });
}

/* ----------------------------- timed_base --------------------------- */

timed_base::timed_base()
  : time_()
{
}

void
timed_base::commit()
{
  time_ += time_type(do_commit(timed_duration::zero_time));
}

void
timed_base::commit(time_type const& until)
{
  if (until > time_) {
    commit(duration_type(until - time_));
  } else {
    // TODO: add warning?
    commit();
  }
}

void
timed_base::commit(duration_type const& duration)
{
  time_ += time_type(do_commit(duration));
}

timed_base::duration_type
timed_base::do_commit(duration_type duration)
{
  return duration;
}

/* ---------------------------- named_object -------------------------- */
#ifdef SYSX_NO_SYSTEMC

named_object::named_object(const char* name)
  : name_(name)
{
  if (object_registry.find(name) != object_registry.end()) {
    SYSX_REPORT_FATAL(sysx::report::plain_msg)
      << "timed_object " << name << " already defined.";
  }
  object_registry[name] = this;
  std::cout << "registered " << name << "\n";
}

named_object::~named_object()
{
  object_registry.erase(name());
}

const char*
named_object::name() const
{
  return name_.c_str();
}

const char*
named_object::kind() const
{
  return "object";
}

#endif // SYSX_NO_SYSTEMC

} // namespace tracing

/* Taf!
 */
