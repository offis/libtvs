/*
 * Copyright (c) 2017-2018 OFFIS Institute for Information Technology
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
 * physical unit related library (error) messages.
 *
 * It is assumed, that this file contains only invocations
 * of the SYSX_REPORT_DEFINE_MSG_() macro, which maps message ids
 * to their library subsystem and an appropriate message prefix.
 *
 * \see report.h
 *
 */

#ifndef SYSX_UNITS_REPORT_MSGS_H_INCLUDED_
#define SYSX_UNITS_REPORT_MSGS_H_INCLUDED_

#include <tvs/utils/report/macros_def.h>
#include <tvs/utils/report/report_base.h>

namespace sysx {
SYSX_REPORT_BEGIN_DEFINITION_

// SYSX_REPORT_DEFINE_MSG_( Type, Id [, MsgTemplate] )

/* ---------------- unit library --------------- */

SYSX_REPORT_DEFINE_MSG_(invalid_unit_symbol,
                        "Units/Invalid unit",
                        "Invalid unit symbol '%s' detected, "
                        "expected (prefixed) %s unit ('%s')");

SYSX_REPORT_DEFINE_MSG_(invalid_prefix_symbol,
                        "Units/Invalid prefix",
                        "Invalid unit prefix symbol '%s' detected");

SYSX_REPORT_END_DEFINITION_
} // namespace sysx

#include <tvs/utils/report/macros_undef.h>

#endif // SYSX_UNITS_REPORT_MSGS_H_INCLUDED_
/* Taf!
 * :tag: (units,s)
 */
