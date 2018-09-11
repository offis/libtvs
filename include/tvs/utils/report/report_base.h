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

#ifndef SYSX_UTILS_REPORT_BASE_H_INCLUDED_
#define SYSX_UTILS_REPORT_BASE_H_INCLUDED_

/**
 * \file  report_base.h
 * \brief debugging and error handling
 *
 * This file contains the implementation of the basic debugging and
 * error handling utilities for the COMPLEX library.
 *
 * \see sysx::report
 */
#include "tvs/utils/report/message.h"

#include <sstream>
#include <string>

namespace sysx {
namespace report {

/**
 * \brief COMPLEX error message severities
 *
 * Internal enumeration used for error handling.
 * The values correspond to those defined in SystemC' sc_severity
 *
 * \see sc_core::sc_severity
 */
enum report_severity
{
  SYSX_SVRTY_FATAL_ = 0,   ///< unrecoverable errors, \see SC_FATAL
  SYSX_SVRTY_ERROR_ = 1,   ///< error, maybe recoverable, \see SC_ERROR
  SYSX_SVRTY_WARNING_ = 2, ///< possible problem, \see SC_WARNING
  SYSX_SVRTY_INFO_ = 4,    ///< informational message, \see SC_INFO
  SYSX_SVRTY_DEBUG_ = 8,   ///< debugging output (without prefix)
  /// largest severity value
  SYSX_SVRTY_LAST_
};

/* ----------------------- report helper class ------------------------ */

/**
 * \class report_base
 * \brief error and debug messages
 *
 * This class handles the error reporting and debugging
 * of the COMPLEX library.  It should not be used directly.
 * Use the various provided macros instead.
 *
 * \see SYSX_REPORT_DEFINE_MSG_(), SYSX_REPORT_FATAL(),
 *      SYSX_REPORT_ERROR(), SYSX_REPORT_WARNING(),
 *      SYSX_REPORT_INFO(), SYSX_MESSAGE()
 */
class report_base
{
protected:
  typedef report_base base_type;

  static const char* const file_unknown; ///< used, if __FILE__ macro is missing
  static const int line_unknown;         ///< used, if __LINE__ macro is missing
  static const char* const lib_prefix;   ///< prefix for message ids

public:
  /// shortcut for report severity
  typedef report_severity severity;

  /// report-type indicator
  typedef struct tag
  {
  } is_report_type;

protected:
  /**
   * \brief constructor to prepare report
   *
   * This constructor is provided to set up a new report. It is
   * used within the reporting macros to pass the current
   * file and line to the report.
   *
   * \param msg_tpl The report's basic message template. Can contain
   *                "%s" placeholders, that can then be filled via the
   *                % operator.
   *
   * \param s       The report's severity, see report_severity
   * \param file    The current file, usually called with __FILE__ macro.
   * \param line    The current line, usually called with __LINE__ macro.
   *
   * \see ~report_base(), SYSX_REPORT_DEFINE_MSG_()
   */
  explicit report_base(const char* const msg_tpl,
                       severity s = SYSX_SVRTY_DEBUG_,
                       const char* const file = file_unknown,
                       int line = line_unknown);

public:
  /**
   * \brief stream helper
   *
   * This method returns a stream, that can be conveniently
   * used to append more information to the current report.
   *
   * \param  context  This parameter can be used with macros
   *                  like __PRETTY_FUNCTION__. If it is given,
   *                  the current simulation context (calling
   *                  function, current process, simulation time)
   *                  is added to the report.
   * \see    SYSX_MESSAGE(), reduce_function()
   *
   * \return stream_t   reference to output stream
   * \see ~report_base
   */
  message& fill(const char* context = nullptr);

  /**
   * \brief issue report
   *
   * This method passes the current report to the SystemC error
   * reporting facilities.  If a message is pending, this method
   * is called during destruction of this instance, at the latest.
   *
   * \warning This method may throw an exception, depending on the
   *          current sc_core::sc_actions set for the current message
   *          ID.
   */
  void trigger() const;

  /**
   * \brief cancel current report
   *
   * This method disables the current report. It will not trigger
   * automatically on destruction.
   *
   * \note Cancellation can not be revoked.
   * \see trigger()
   */
  void cancel();

protected:
  /**
   * \brief return current report ID
   *
   * This method dispatches to the static implementation
   * of the raw_id() method in the derived classes.
   *
   * \see SYSX_REPORT_DEFINE_MSG_()
   */
  virtual const char* get_id() const = 0;

  /**
   * \brief destructor (fires report, if necessary)
   *
   * If the current report object represents a  pending report,
   * it is passed through to the SystemC error reporting on destruction.
   *
   * \warning This destructor may throw an exception, if SystemC
   *          error reporting does for the represented report.
   *          Do NOT use this class as a member unless you know
   *          what you are doing.
   */
  virtual ~report_base();

  /** disabled copy constructor */
  report_base(const report_base&);

private:
  /**
   * \brief function name reduction helper
   *
   * This method reduces a string given by macros like
   * __PRETTY_FUNCTION__ to a shorter fraction: "methodname()".
   * It is used, if the \c context parameter is given to
   * the stream() method.
   *
   */
  static std::string reduce_function(const char*);

  message msg_;
  bool active_;
  severity sev_;
  const char* const file_;
  unsigned int line_;
  const char* id_;

}; /* class report_base */

} /* namespace report */
} /* namespace sysx  */

#endif /* SYSX_UTILS_REPORT_BASE_H_INCLUDED_ */
/* Taf!
 * :tag: (utils,h)
 */
