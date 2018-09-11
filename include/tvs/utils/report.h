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

#ifndef SYSX_UTILS_REPORT_H_INCLUDED_
#define SYSX_UTILS_REPORT_H_INCLUDED_

#include "tvs/utils/macros.h"

/**
 * \file  report.h
 * \brief reporting errors, warnings and infos
 *
 * This file contains the internal error-handling macros
 * for the SYSX library.  These facilities basically
 * provide a thin layer on top of the standardised reporting
 * API of SystemC.
 *
 */

namespace sysx {

/**
 * \namespace sysx::report
 * \brief namespace wrapper for the error/debug reporting utilities
 *
 * This namespace encapsulates the error-handling and reporting
 * facilities in COMPLEX.  In particular, this includes the
 * different report-id classes, which are derived from the
 * report_base class.
 *
 * \see report_base
 *
 * The reporting itself is done through various macros.
 * A basic printf()-like placefolder syntax is supported.
 *
 * \see SYSX_REPORT_DEFINE_MSG_(),
 *      SYSX_REPORT_FATAL(),
 *      SYSX_REPORT_ERROR(), SYSX_REPORT_WARNING(),
 *      SYSX_REPORT_INFO(),  SYSX_DEBUG_MSG(), SYSX_DEBUG_INFO
 *
 * All reporting classes provide a static \c id function, that
 * can be used to filter/modify the behaviour of the SystemC
 * kernel to the reports:
 *
 * \code
 *   sc_core::sc_report_handler::set_actions
 *      ( some_warning_class::id(), sc_core::SC_DO_NOTHING );
 * \endcode
 *
 */
namespace report { /* for documentation only */
}

} // namespace sysx

#include "tvs/utils/macros.h"
#include "tvs/utils/report/report_base.h"
#include "tvs/utils/report/report_msgs.h"

/* ------------------------- reporting macros ------------------------- */

/**
 * \def SYSX_MESSAGE()
 * \brief conditional message
 *
 * This macro can be used to conditionally report raw messages.
 * The macro can be used as a simple stream, with a condition parameter,
 * that decides if the report should be issued.
 *
 * Additionally, the current simulation time, the calling process,
 * and the current method name are printed, too.
 *
 * \param ConditionFlag If ConditionFlag is \c true, the following message
 *                      is reported.
 *
 * Example usage:
 * \code
 *   SYSX_MESSAGE( my_condition )
 *     << "The condition is true.";
 * \endcode
 *
 * \hideinitializer
 */
#define SYSX_MESSAGE(ConditionFlag, ...)                                       \
  if (!(ConditionFlag)) {                                                      \
    ((void)0);                                                                 \
  } else                                                                       \
    ::sysx::impl::report<::sysx::report::plain_msg>::type().fill(              \
      __PRETTY_FUNCTION__)                                                     \
      << "" __VA_ARGS__

/**
 * \def SYSX_REPORT_FATAL
 * \brief report fatal errors
 *
 * This macro can be used to report fatal, non-recoverable errors.
 * The given report id needs to be present in report_msgs.h (or
 * another included collection of error-messages).
 * As a result, a fatal error is reported to the SystemC kernel's
 * error handling facilities.
 *
 * Example usage:
 * \code
 *   SYSX_REPORT_FATAL( sysx::report::internal_error )
 *     << "This is a fatal error.";
 * \endcode
 *
 * \param ID Type of the report (derived from report_base)
 * \see SYSX_SVRTY_FATAL_, report_msgs.h
 *
 * \hideinitializer
 */
#define SYSX_REPORT_FATAL(ID)                                                  \
  SYSX_IMPL_REPORT_STREAM_(::sysx::report::SYSX_SVRTY_FATAL_, ID)

/**
 * \def SYSX_ABORT
 * \brief terminate simulation
 * \deprecated Use a proper report ID instead.
 * \hideinitializer
 */
#define SYSX_ABORT()                                                           \
  SYSX_REPORT_FATAL(sysx::report::abort_called);                               \
  std::unexpected() /* avoid warnings about missing return */

/**
 * \def SYSX_REPORT_ERROR
 * \brief report errors
 *
 * This macro can be used to report "regular" errors.
 * The given report id needs to be present in report_msgs.h (or
 * another included collection of error-messages).
 * As a result, an error is reported to the SystemC kernel's
 * error handling facilities.
 *
 * Example usage:
 * \code
 *   SYSX_REPORT_ERROR( sysx::report::internal_error )
 *     << "This is an error.";
 * \endcode
 *
 * \param id Type of the report (derived from report_base)
 * \see SYSX_SVRTY_ERROR_, report_msgs.h
 *
 * \hideinitializer
 */
#define SYSX_REPORT_ERROR(id)                                                  \
  SYSX_IMPL_REPORT_STREAM_(::sysx::report::SYSX_SVRTY_ERROR_, id)

/**
 * \def SYSX_REPORT_WARNING
 * \brief report warnings
 *
 * This macro can be used to report warning messages.
 * The given report id needs to be present in report_msgs.h (or
 * another included collection of error-messages).
 * As a result, a warning is reported to the SystemC kernel's
 * error handling facilities.
 *
 * Example usage:
 * \code
 *   SYSX_REPORT_WARNING( sysx::report::plain_msg )
 *     << "This is a warning.";
 * \endcode
 *
 * \param id Type of the report (derived from report_base)
 * \see SYSX_SVRTY_WARNING_, report_msgs.h
 *
 * \hideinitializer
 */
#define SYSX_REPORT_WARNING(id)                                                \
  SYSX_IMPL_REPORT_STREAM_(::sysx::report::SYSX_SVRTY_WARNING_, id)

/**
 * \def SYSX_REPORT_INFO
 * \brief report informational messages
 *
 * This macro can be used to report informational messages.
 * The given report id needs to be present in report_msgs.h (or
 * another included collection of error-messages).
 * As a result, an informational message is reported to the SystemC
 * kernel's error handling facilities.
 *
 * Example usage:
 * \code
 *   SYSX_REPORT_INFO( sysx:report::plain_msg )
 *     << "This is an info.";
 * \endcode
 *
 * \note SYSX_MESSAGE() can be used for contextual
 *       information output, based on an additional condition.
 *
 * \param id Type of the report (derived from report_base)
 * \see SYSX_SVRTY_INFO_, report_msgs.h
 *
 * \hideinitializer
 */
#define SYSX_REPORT_INFO(id)                                                   \
  SYSX_IMPL_REPORT_STREAM_(::sysx::report::SYSX_SVRTY_INFO_, id)

/**
 * \def SYSX_IMPL_REPORT_STREAM_()
 * \brief internal reporting macro
 *
 * This helper macro is used by the public reporting macros
 * to really issue the requested report at the appropriate
 * severity.
 *
 * \note It should not be used directly. Use the severity
 *       specific macros SYSX_REPORT_FATAL(),
 *       SYSX_REPORT_ERROR() etc. instead.
 *
 * \see sysx::report::report_severity
 *
 * \hideinitializer
 */
#define SYSX_IMPL_REPORT_STREAM_(Severity, ID)                                 \
  /* create temporary report object and return its stream */                   \
  ::sysx::impl::report<ID>::type(Severity, __FILE__, __LINE__)                 \
    .fill(__PRETTY_FUNCTION__)

namespace sysx {
namespace impl {

/**
 * \brief internal reporting helper template
 *
 * This template class is used, to statically check the given
 * error ID in the various reporting macros.
 *
 * \note It should not be used directly. Use the severity
 *       specific macros SYSX_REPORT_FATAL(),
 *       SYSX_REPORT_ERROR() etc. instead.
 *
 */
template<typename ReportType,
         typename Valid = typename ReportType::is_report_type>
struct report;

template<typename ReportType>
struct report<ReportType, sysx::report::report_base::tag>
{
  typedef ReportType type;
  static const char* id() { return type::id(); }
};

} // namespace impl
} // namespace sysx

#endif /* SYSX_UTILS_REPORT_H_INCLUDED_ */
/* Taf!
 * :tag: (utils,h)
 */
