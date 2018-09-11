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

#ifndef SYSX_UTILS_REPORT_MACROS_DEF_H_INCLUDED_
#define SYSX_UTILS_REPORT_MACROS_DEF_H_INCLUDED_

/// mark beginning of report definition block
#define SYSX_REPORT_BEGIN_DEFINITION_ namespace report {

/// mark end of report definition block
#define SYSX_REPORT_END_DEFINITION_ } /* namespace report */

/// SysX library name and message prefix
#ifndef SYSX_LIBRARY_NAME
#define SYSX_LIBRARY_NAME "SysX"
#endif
#define SYSX_IMPL_REPORT_LIBRARY_PREFIX_ "/" SYSX_LIBRARY_NAME "/"

/**
 * \def SYSX_REPORT_DEFINE_MSG_
 * \brief report type definition helper
 *
 * Internal macro that is recommended to be used for
 * the definition of new report types. This macro results
 * in the definition of a new class, derived from
 * report::report_base.
 *
 * \param Type        class name of newly defined report type
 * \param Id          string to identify report type through
 *                    SystemC report handling
 * \param MsgTemplate Template for message creation.
 *
 * The message template can contain "%s" placeholders, that
 * can be filled via the % operator during the message creation.
 *
 * Example:
 * \code
 *
 * SYSX_REPORT_DEFINE_MSG_( example_rpt, "Example/Category",
 *                          "This is a %s template!\n"
 *                          "Replacers are type-safe: %s"
 * );
 * //...
 * SYSX_REPORT_INFO( sysx::report::example_rpt )
 *   % "fancy"               // fill placeholder
 *   << "Some more text."    // append some text
 *   % something_streamable; // fill another placeholder (from a variable)
 *
 * \endcode
 *
 * The given \c Id can be accessed via the static \c id() function
 * in each reporting class.  This function can be used to to
 * filter/modify the behaviour of the SystemC kernel to the report:
 *
 * \code
 *   sc_core::sc_report_handler::set_actions
 *      ( sysx::report::example_rpt::id(), sc_core::SC_DO_NOTHING );
 * \endcode
 *
 * \see report_base, report_msgs.h
 * \hideinitializer
 *
 */
#define SYSX_REPORT_DEFINE_MSG_(Type, Id, MsgTemplate)                         \
  struct Type : ::sysx::report::report_base                                    \
  {                                                                            \
    typedef report_base base_type;                                             \
    SYSX_REPORT_IMPL_CTOR_(Type, MsgTemplate "")                               \
    static const char* id() { return SYSX_IMPL_REPORT_LIBRARY_PREFIX_ Id; }    \
    static const char* raw_id() { return Id; }                                 \
                                                                               \
  protected:                                                                   \
    const char* get_id() const { return Type::raw_id(); }                      \
  }

/**
 * \brief internal helper macro
 * \hideinitializer
 */
#define SYSX_REPORT_IMPL_CTOR_(Type, MsgTemplate)                              \
  Type()                                                                       \
    : base_type(MsgTemplate)                                                   \
  {}                                                                           \
  Type(base_type::severity s)                                                  \
    : base_type(MsgTemplate, s)                                                \
  {}                                                                           \
  Type(base_type::severity s, const char* const file, int line)                \
    : base_type(MsgTemplate, s, file, line)                                    \
  {}

#endif /* SYSX_UTILS_REPORT_MACROS_DEF_H_INCLUDED_ */
/* Taf!
 * :tag: (utils,h)
 */
