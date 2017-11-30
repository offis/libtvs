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
 * \file   object_host.cpp
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>
 * \brief  Implementaion of the object host interface (for SystemC and generic)
 */

#include "tvs/tracing/object_host.h"
#include "tvs/tracing/timed_stream_base.h"

#include "tvs/utils/systemc.h"

#include "tvs/utils/report.h"
#include "tvs/tracing/report_msgs.h"

#include <functional>

namespace tracing {

namespace {
host::sync_fn_type sync_fn;
} // anonymous namespace

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
      << "Please specify a sync function with "
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
  SYSX_REPORT_FATAL(sysx::report::not_implemented)
    % "scope traversal";
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
  // TODO: implement for non-SystemC
  SYSX_REPORT_FATAL(sysx::report::not_implemented) % "unique name generation";
  return name;
#else
  return sc_core::sc_gen_unique_name(name);
#endif // SYSX_NO_SYSTEMC
}

tracing::timed_stream_base*
lookup(const char* name)
{
#ifdef SYSX_NO_SYSTEMC
  SYSX_REPORT_FATAL(sysx::report::not_implemented) % "stream lookup";
  return nullptr;
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

} // namespace tracing
