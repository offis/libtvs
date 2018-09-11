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
 * \brief utilities error messages
 *
 * It is assumed, that this file contains only invocations
 * of the SYSX_REPORT_DEFINE_MSG_() macro, which maps message ids
 * to their library subsystem and an appropriate message prefix.
 *
 * \see report.h
 *
 */

#ifndef SYSX_UTILS_REPORT_MSGS_H_INCLUDED_
#define SYSX_UTILS_REPORT_MSGS_H_INCLUDED_

#include "tvs/utils/report/macros_def.h"
#include "tvs/utils/report/report_base.h"

namespace sysx {
namespace utils {
SYSX_REPORT_BEGIN_DEFINITION_

// SYSX_REPORT_DEFINE_MSG_( Type, Id [, MsgTemplate] )

SYSX_REPORT_DEFINE_MSG_(
  name_already_used,
  "utils/shared_attribute/name already used",
  "Name '%s' already used for different type than type '%s'");

SYSX_REPORT_DEFINE_MSG_(no_owner,
                        "utils/shared_attribute/no owner",
                        "Current context is no sc_object. Shared attributes "
                        "can only be appended to sc_objects");

SYSX_REPORT_DEFINE_MSG_(variant_error,
                        "utils/variant",
                        "error in variant type handling");

SYSX_REPORT_DEFINE_MSG_(registered,
                        "utils/factory/registered",
                        "The %s '%s' was registered");

SYSX_REPORT_DEFINE_MSG_(not_registered,
                        "utils/factory/not registered",
                        "'%s' is not a registered %s");

SYSX_REPORT_DEFINE_MSG_(registered_twice,
                        "utils/factory/registered twice",
                        "Can not register two %ss with the same name '%s'");

SYSX_REPORT_END_DEFINITION_
} // namespace utils
} // namespace sysx

#include "tvs/utils/report/macros_undef.h"

#endif // SYSX_UTILS_REPORT_MSGS_H_INCLUDED_
//! \todo: fix exposure as public header again
/* Taf!
 * :tag: (utils,h)
 */
