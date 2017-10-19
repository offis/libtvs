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
 * \file   timed_writer_base.cpp
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed value writer for tracing (writer base implementation)
 * \see    timed_stream.h
 */

#include "tvs/tracing/timed_writer_base.h"

#include "tvs/utils/assert.h"
#include "tvs/utils/report.h"

#include "report_msgs.h"

namespace tracing {

timed_writer_base::timed_writer_base(stream_type* own_stream)
  : stream_()
  , own_stream_(own_stream)
{
}

timed_writer_base::~timed_writer_base()
{
  detach();
  delete own_stream_;
}

/* ---------------------------- attach/detach -------------------------- */

void
timed_writer_base::attach(const char* name)
{
  timed_stream_base* str = timed_stream_base::lookup(name);
  if (!str) {
    sc_core::sc_object* scope = sc_core::sc_get_current_object();
    if (scope) {
      std::stringstream lname;
      lname << scope->name() << sc_core::SC_HIERARCHY_CHAR << name;
      str = timed_stream_base::lookup(lname.str().c_str());
    }

    if (!str) {
      SYSX_REPORT_ERROR(report::stream_lookup) % name
        << "object not found "
        << "(scope: " << (scope ? scope->name() : "<top>") << ")";
      return;
    }
  }

  this->attach(*str);
}

void
timed_writer_base::attach(timed_stream_base& stream)
{
  if (stream_) {
    SYSX_REPORT_ERROR(report::stream_attach) % stream.name()
      << "writer is already attached to stream "
         "'"
      << stream_->name() << "'";
    return;
  }
  stream_ = &stream;
  stream_->attach(*this);
}

void
timed_writer_base::detach()
{
  if (!stream_)
    return;
  stream_->detach(*this);
  stream_ = NULL;
}

} // namespace tracing

/* Taf!
 */
