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
 * \file report_msgs.h
 *
 * This file contains information about the different,
 * tracing related library (error) messages.
 *
 * It is assumed, that this file contains only invocations
 * of the SYSX_REPORT_DEFINE_MSG_() macro, which maps message ids
 * to their library subsystem and an appropriate message prefix.
 *
 * \see report.h
 */
#ifndef TVS_REPORT_MSGS_H_INCLUDED_
#define TVS_REPORT_MSGS_H_INCLUDED_

#include <tvs/utils/report/macros_def.h>
#include <tvs/utils/report/report_base.h>

namespace tracing {
SYSX_REPORT_BEGIN_DEFINITION_

// SYSX_REPORT_DEFINE_MSG_( Type, Id [, MsgTemplate] )

SYSX_REPORT_DEFINE_MSG_(no_stream_to_sync,
                        "tracing/stream sync",
                        "Could not find a stream in this component");
SYSX_REPORT_DEFINE_MSG_(stream_attach,
                        "tracing/stream attach",
                        "attach() failed for stream '%s'");

SYSX_REPORT_DEFINE_MSG_(stream_detach,
                        "tracing/stream detach",
                        "detach() failed for stream '%s'");

SYSX_REPORT_DEFINE_MSG_(stream_lookup,
                        "tracing/stream lookup",
                        "stream lookup failed for '%s'");

SYSX_REPORT_DEFINE_MSG_(
  header_written,
  "tracing/stream processor",
  "add() called after processor '%s' received input tokens");

SYSX_REPORT_END_DEFINITION_
} // namespace tracing

#include <tvs/utils/report/macros_undef.h>

#endif // TVS_REPORT_MSGS_H_INCLUDED_
/* Taf!
 */
