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

// NO "REAL" INCLUDE GUARDS - multiple inclusion allowed

/**
 * \file  assert.h
 * \brief run-time assertions
 *
 * This file contains the SYSX_ASSERT() macro for run-time
 * assertions. This is similar to the C-style assert() macro,
 * and even more similar to the SystemC macro sc_assert().
 *
 * \todo add support for compile-time assertions
 *
 * This file can be included multiple times.  Each time, the
 * SYSX_ASSERT() macro is activated or deactivated according
 * to the SYSX_NDEBUG switch.
 *
 * With this mechanism, certain performance critical sections
 * can be wrapped in a block like in the following example.
 *
 * Example:
 * \code
 *
 * // this section is performance critical
 * #if !defined( SYSX_DEBUG )
 * #  define SYSX_NDEBUG // disable assertions in non-debug build
 * #endif
 * #include "tvs/utils/assert.h"
 *
 * void performance_critical_code_with_assertions()
 * {
 *   //...
 *   SYSX_ASSERT( false ); // no check here -> no error reported
 * }
 *
 * #undef SYSX_NDEBUG // re-enable assertions
 * #include "tvs/utils/assert.h"
 *
 * \endcode
 *
 * \note This file depends on "report.h" being included
 *       separately, if assertions are enabled.
 *       It is NOT included from this file, though.  This allows the
 *       inclusion of this file at any place in the source code.
 *
 * \see sysx::report, SYSX_REPORT_FATAL(), sc_assert(), assert()
 */

#ifdef SYSX_UTILS_ASSERT_H_INCLUDED_

#undef SYSX_ASSERT
#undef SYSX_UTILS_ASSERT_H_INCLUDED_

#endif

#define SYSX_UTILS_ASSERT_H_INCLUDED_

/* ---------------------------- assertions ---------------------------- */

#if !defined(SYSX_NDEBUG) && !defined(SYSX_SYNTHESIS_)

/**
 * \def   SYSX_ASSERT()
 * \brief run-time assertions
 *
 * This macro can be used for C-style run-time assertions.
 * The assertions are omitted, if the preprocessor symbol \c SYSX_NDEBUG
 * is defined during the inclusion of the assert.h header file.
 *
 * Otherwise, if the given expression is \c false, a fatal error
 * is reported via an SYSX_REPORT_FATAL() call.
 *
 * \param expr  expression, that is required to be true.
 *
 * Example usage:
 * \code
 *   // ...
 *   SYSX_ASSERT( ultimate_answer == 42 ); // should hold...
 * \endcode
 *
 * \warning Do \em NOT rely on any side-effects within the
 *          asserted expression, since these might be elided in an
 *          optimised build!
 *
 * \note    You should only check internal invariants using this
 *          assertion mechanism.  Errors, that are expected (e.g. due
 *          to invalid user-code) should be checked explicitly and
 *          reported through the regular error-handling mechanisms.
 *
 * \see assert.h, SYSX_REPORT_FATAL()
 */
#define SYSX_ASSERT(expr)                                                      \
  ((void)(sysx_likely(expr)                                                    \
            ? 0                                                                \
            : ((SYSX_REPORT_FATAL(::sysx::report::assertion_failed) % #expr),  \
               0)))

#else // assertions are disabled

#define SYSX_ASSERT(expr) ((void)0) /* ignore */

#endif /* SYSX_NDEBUG */

/* Taf!
 * :tag: (utils,H)
 */
