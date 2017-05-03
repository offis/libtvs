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

#ifndef SYSX_UTILS_DEBUG_H_INCLUDED_
#define SYSX_UTILS_DEBUG_H_INCLUDED_

#include "tvs/utils/report.h"
#include "tvs/utils/report/macros_def.h"

/**
 * \def SYSX_DEBUG_MSG
 * \brief print debug messages to the stdout
 *
 * This macro can be used to print debug messages to the standard
 * output.  The SystemC reporting mechanism is always bypassed in
 * this case.
 *
 * The given debug level can be used to filter out messages at
 * compile-time, according to the value of SYSX_DEBUG_LEVEL.
 * No further filtering supported here.
 *
 * \b Example:
 * \code
 *    SYSX_DEBUG_MSG(5, "Hello %s" )
 *      % "world";
 * \endcode
 *
 * \param Level       debug level, see \ref SYSX_DEBUG_LEVEL
 * \param MsgTemplate message template, with optional \c %s paceholders
 *
 * \see SYSX_DEBUG_INFO, SYSX_REPORT_INFO
 * \hideinitializer
 */
#define SYSX_DEBUG_MSG(Level, MsgTemplate)                                     \
  if (Level > SYSX_DEBUGLEVEL) {                                               \
    (void)(0);                                                                 \
  } else                                                                       \
    ::sysx::impl::report<::sysx::report::debug_msg<Level>>::type(              \
      MsgTemplate "", __FILE__, __LINE__)                                      \
      .fill()

/**
 * \def SYSX_DEBUG_INFO
 * \brief report debug messages to SystemC
 *
 * This macro can be used to report debug messages to the standard
 * SystemC reporting mechanism as SC_INFO messages.
 * this case.
 *
 * The given debug level can be used to filter out messages at
 * compile-time, according to the value of SYSX_DEBUG_LEVEL
 * (\ref SYSX_DEBUG_MSG).  Additionally, the regular SystemC
 * filtering mechanisms can be used during run-time:
 *
 * \b Example:
 * \code
 *    sc_core::sc_report_handler::set_actions
 *      ( sysx::report::debug_msg<5>::id(), sc_core::SC_DO_NOTHING );
 *
 *    SYSX_DEBUG_INFO(5)
 *      << "Hello world!";
 * \endcode
 *
 * \param Level debug level, see \ref SYSX_DEBUG_LEVEL
 *
 * \todo  Add support for verbosity control in SystemC 2.3
 *
 * \see SYSX_DEBUG_INFO, SYSX_REPORT_INFO
 * \hideinitializer
 */
#define SYSX_DEBUG_INFO(Level)                                                 \
  if (Level > SYSX_DEBUGLEVEL) {                                               \
    (void)(0);                                                                 \
  } else                                                                       \
    SYSX_REPORT_INFO(sysx::report::debug_msg<Level>)

namespace sysx {
SYSX_REPORT_BEGIN_DEFINITION_

/// internal class for debug messages
template<unsigned Level>
struct debug_msg : report_base
{
  typedef report_base base_type;

  /** \name \ref SYSX_DEBUG_MSG constructors */
  ///\{
  explicit debug_msg(const char* msg_template = "")
    : base_type(msg_template, severity(SYSX_SVRTY_DEBUG_ | Level))
  {
  }

  debug_msg(const char* msg_template, const char* const file, int line)
    : base_type(msg_template, severity(SYSX_SVRTY_DEBUG_ | Level), file, line)
  {
  }
  ///\}

  /** \name \ref SYSX_DEBUG_INFO constructors */
  ///\{
  explicit debug_msg(base_type::severity sev)
    : base_type("", sev)
  {
  }

  debug_msg(base_type::severity sev, const char* const file, int line)
    : base_type("", sev, file, line)
  {
  }
  ///\}

  static const char* id()
  {
    static char id_str[] = SYSX_IMPL_REPORT_LIBRARY_PREFIX_ "Debug/0";
    static bool adjust =
      ((id_str[sizeof(id_str) - 2] += (Level % 10)), Level < 8);
    return (adjust) ? id_str : SYSX_IMPL_REPORT_LIBRARY_PREFIX_ "Debug/8";
  }

  static const char* raw_id()
  {
    static char id_str[] = "Debug/0";
    static bool adjust =
      ((id_str[sizeof(id_str) - 2] += (Level % 10)), Level < 8);
    return (adjust) ? id_str : "Debug/8";
  }

protected:
  const char* get_id() const { return debug_msg::raw_id(); }
};

SYSX_REPORT_END_DEFINITION_
} // namespace sysx

#include "tvs/utils/report/macros_undef.h"

#endif /* SYSX_UTILS_DEBUG_H_INCLUDED_ */
/* Taf!
 * :tag: (utils,h)
 */
