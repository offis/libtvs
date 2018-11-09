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

#include "tvs/utils/report/report_base.h"

#include "tvs/utils/macros.h"

#include "tvs/utils/assert.h"
#include "tvs/utils/report.h"

#include "tvs/utils/report/macros_def.h"

#ifndef SYSX_NO_SYSTEMC
#include "tvs/utils/systemc.h"
#else
#include <iostream>
#include <stdexcept>
#endif

namespace sysx {
namespace report {

#if not defined(SYSX_NO_SYSTEMC)
/// local severity conversion function
inline static sc_core::sc_severity
sysx_severity_to_sc_severity(report_base::severity svrty);
#endif

report_base::report_base(const char* const msg_tpl,
                         severity s,
                         const char* const file,
                         int line)
  : msg_(msg_tpl)
  , active_(true)
  , sev_(s)
  , file_(file)
  , line_(line)
  , id_("<unknown>")
{}

message&
report_base::fill(const char* context)
{
  // cache current ID - would be lost during trigger() called from destructor
  id_ = this->get_id();
  // include simulation context in message
  if (context) {
    msg_ << "\n";
#if not defined(SYSX_NO_SYSTEMC)
    msg_ << "@" << ::sc_core::sc_time_stamp() << "~"
         << ::sc_core::sc_delta_count() << " "
         << sc_core::sc_get_current_process_handle().name();
#endif
    msg_ << " in function " << reduce_function(context);
  }
  return msg_;
}

void
report_base::cancel()
{
  active_ = false;
}

report_base::~report_base()
{
  // cancel current message, another exception is on its way
  if (std::uncaught_exception()) {
    cancel();
  }
  this->trigger();
}

void
report_base::trigger() const
{
  if (sysx_likely(active_)) {
    using std::cerr;
    using std::cout;
    using std::endl;

#if 1
    // is it a debug message?
    if (sev_ & SYSX_SVRTY_DEBUG_) {
      unsigned level = (sev_ ^ SYSX_SVRTY_DEBUG_);
      if (level) { // write prefix, only if level is set
        cout << lib_prefix << id_ << ": ";
      }
      // dump message
      cout << msg_.combine();

      if (level && file_ != file_unknown)
        cout << "\n(file: " << file_ << ", line: " << line_ << ")";

      cout << endl;
    } else
#endif
    { // regular message

#if not defined(SYSX_NO_SYSTEMC)

      sc_core::sc_severity scs = sysx_severity_to_sc_severity(sev_);

      std::string msg_type(lib_prefix);
      msg_type += id_;

      // call SystemC report handler
      sc_core::sc_report_handler::report(
        scs, msg_type.c_str(), msg_.combine().c_str(), file_, line_);

#else // SYSX_NO_SYSTEMC -> print/throw run-time error

      std::stringstream what;
      std::ostream* out = NULL;
      bool throw_msg = false;

      switch (sev_) {
        case SYSX_SVRTY_FATAL_:
        case SYSX_SVRTY_ERROR_:
        default:
          out = &what;
          throw_msg = true;
          break;
        case SYSX_SVRTY_WARNING_:
          out = &std::cerr;
          break;
        case SYSX_SVRTY_INFO_:
        case SYSX_SVRTY_DEBUG_:
          out = &std::cout;
          break;
      }

      *out << lib_prefix << id_ << ": " << msg_.combine();

      if (file_ != file_unknown)
        *out << "\n(file: " << file_ << ", line: " << line_ << ")";

      if (throw_msg)
        throw std::runtime_error(what.str());

#endif // SYSX_NO_SYSTEMC
    }
  }
}

#if not defined(SYSX_NO_SYSTEMC)
inline static sc_core::sc_severity
sysx_severity_to_sc_severity(report_base::severity svrty)
{
  // match remaining debug messages to INFO severity
  if (svrty & SYSX_SVRTY_DEBUG_)
    svrty = SYSX_SVRTY_DEBUG_;

  // translate to SystemC severity
  sc_core::sc_severity scs = ::sc_core::SC_MAX_SEVERITY;
  switch (svrty) {
#define SYSX_TRANSLATE_RPT_SVRTY_HELPER(sysx, sysc)                            \
  case sysx:                                                                   \
    scs = ::sc_core::sysc;                                                     \
    break

    SYSX_TRANSLATE_RPT_SVRTY_HELPER(SYSX_SVRTY_FATAL_, SC_FATAL);
    SYSX_TRANSLATE_RPT_SVRTY_HELPER(SYSX_SVRTY_ERROR_, SC_ERROR);
    SYSX_TRANSLATE_RPT_SVRTY_HELPER(SYSX_SVRTY_WARNING_, SC_WARNING);
    SYSX_TRANSLATE_RPT_SVRTY_HELPER(SYSX_SVRTY_INFO_, SC_INFO);
    SYSX_TRANSLATE_RPT_SVRTY_HELPER(SYSX_SVRTY_DEBUG_, SC_INFO);
    default:
      SYSX_ASSERT(false && "Invalid report severity!");

#undef SYSX_TRANSLATE_RPT_SVRTY_HELPER
  }
  return scs;
}
#endif // not SYSX_NO_SYSTEMC

std::string
report_base::reduce_function(const char* char_text)
{
  std::string working_copy(char_text);
  // remove all namespace and class names
  std::string::size_type idx_bra = working_copy.find("(");
  std::string::size_type idx_sep = working_copy.rfind("::", idx_bra);
  if (idx_sep != std::string::npos)
    working_copy.erase(0, idx_sep + 2);

  // remove all arguments
  idx_bra = working_copy.find("(");
  if (idx_bra != std::string::npos) {
    working_copy[idx_bra] = '(';
    working_copy.erase(idx_bra + 1);
  }
  return working_copy + ")";
}

const char* const report_base::lib_prefix = SYSX_IMPL_REPORT_LIBRARY_PREFIX_;
const char* const report_base::file_unknown = "<unknown>";
const int report_base::line_unknown = -1;

} /* namespace report */
} /* namespace sysx */

/* Taf!
 * :tag: (utils,s)
 */
