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
 * This file contains information about the different, generic
 * library (error) messages.
 *
 * It is assumed, that this file contains only invocations
 * of the SYSX_REPORT_DEFINE_MSG_() macro, which maps message ids
 * to their library subsystem and an appropriate message prefix.
 *
 * \see report.h
 *
 */

#ifndef SYSX_UTILS_REPORT_REPORT_MSGS_H_INCLUDED_
#define SYSX_UTILS_REPORT_REPORT_MSGS_H_INCLUDED_

#include "tvs/utils/report/macros_def.h"
#include "tvs/utils/report/report_base.h"

namespace sysx {
SYSX_REPORT_BEGIN_DEFINITION_

// SYSX_REPORT_DEFINE_MSG_( Type, Id [, MsgTemplate] )

/* ---------------- core library --------------- */

SYSX_REPORT_DEFINE_MSG_(
  abort_called,
  "Core/Abort called",
  "SYSX_ABORT() has been called due to an unrecoverable error.");

SYSX_REPORT_DEFINE_MSG_(assertion_failed,
                        "Core/Assertion failed",
                        "Expression '%s' is unexpectedly false!\n");

SYSX_REPORT_DEFINE_MSG_(internal_error,
                        "Core/Internal error",
                        "An internal error has occured: %s\n");

SYSX_REPORT_DEFINE_MSG_(not_implemented,
                        "Core/Not implemented",
                        "%s is not implemented yet\n.");

/* ----------------- datatypes ----------------- */

SYSX_REPORT_DEFINE_MSG_(index_out_of_bounds,
                        "Datatype/Index out of bounds",
                        "%s: Invalid field index detected: %s >= %s (=size)!");

/* ---------------- elaboration ---------------- */

SYSX_REPORT_DEFINE_MSG_(called_after_elaboration,
                        "Elaboration/Simulation started",
                        "'%s' called after end of elaboration phase.");

/* -------------------------------------------- */

SYSX_REPORT_DEFINE_MSG_(plain_msg,
                        "Core/Plain",
                        SYSX_IMPL_NOTHING_ /* empty message */
);

SYSX_REPORT_END_DEFINITION_
} // namespace sysx

#include "tvs/utils/report/macros_undef.h"

#endif // SYSX_UTILS_REPORT_REPORT_MSGS_H_INCLUDED_
/* Taf!
 * :tag: (utils,h)
 */
