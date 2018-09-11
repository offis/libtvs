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
 * \file  systemc.h
 * \brief central SystemC include file
 *
 */
#if defined(SYSX_NO_SYSTEMC)
#  if !defined(SYSX_HAVE_SYSTEMC_)
#  define SYSX_HAVE_SYSTEMC_ 0
#  endif // SYSX_HAVE_SYSTEMC_
#else

// include main SystemC header (deliberately outside include guards)
#include <systemc>

#ifndef SYSX_UTILS_SYSTEMC_H_INCLUDED_
#define SYSX_UTILS_SYSTEMC_H_INCLUDED_

#include <tvs/utils/macros.h> // SYSX_MAKE_VERSION

/**
 * \def SYSX_HAVE_SYSTEMC_
 * \brief indicator that SystemC context is available
 *
 * The value is set to the detected SystemC version.
 *
 * Different vendors still need to be discriminated
 * via the specific symbols.
 *
 * \see SYSX_MAKE_VERSION
 *
 * \note If \c SYSX_NO_SYSTEMC is defined (i.e. no SystemC support
 *       available), \c SYSX_HAVE_SYSTEMC_ will be set to \c 0,
 *       and SystemC will not be included.
 *
 * \hideinitializer
 */
//#define SYSX_HAVE_SYSTEMC_

#if defined(IEEE_1666_SYSTEMC) && IEEE_1666_SYSTEMC >= 201101L

#define SYSX_HAVE_SYSTEMC_2_3 SYSTEMC_VERSION
#define SYSX_HAVE_SYSTEMC_                                                     \
  SYSX_MAKE_VERSION(SC_VERSION_MAJOR, SC_VERSION_MINOR, SC_VERSION_PATCH)

#else // pre-1666-2011

#ifdef SYSTEMC_VERSION

#if SYSTEMC_VERSION == 20050714
// we found (OSCI) SystemC kernel 2.1.v1 - at least we think so
#define SYSX_HAVE_SYSTEMC_2_1 SYSTEMC_VERSION
#define SYSX_HAVE_SYSTEMC_ SYSX_MAKE_VERSION(2, 1, 1)

#elif SYSTEMC_VERSION == 20060505
// we found (OSCI) SystemC kernel 2.2.05jun06_beta - at least we think so
#define SYSX_HAVE_SYSTEMC_2_2 SYSTEMC_VERSION
#define SYSX_HAVE_SYSTEMC_ SYSX_MAKE_VERSION(2, 1, 90)

#elif SYSTEMC_VERSION == 20070314
// we found (OSCI) SystemC kernel 2.2.0 - at least we think so
#define SYSX_HAVE_SYSTEMC_2_2 SYSTEMC_VERSION
#define SYSX_HAVE_SYSTEMC_ SYSX_MAKE_VERSION(2, 2, 0)

#elif SYSTEMC_VERSION == 20120610
// we found (OSCI) SystemC kernel 2.3.0_pub_rev_20120610
// - at least we think so
//   (should not happen, since IEEE_1666_SYSTEMC is defined)
#define SYSX_HAVE_SYSTEMC_2_3 SYSTEMC_VERSION
#define SYSX_HAVE_SYSTEMC_ SYSX_MAKE_VERSION(2, 2, 90)

#else // SYSTEMC_VERSION unknown

#define SYSX_HAVE_SYSTEMC_ 1

#endif // SYSTEMC_VERSION unknown

#else // ! defined SYSTEMC_VERSION (but <systemc> has been found)

#define SYSX_HAVE_SYSTEMC_ 1

#endif // ! defined SYSTEMC_VERSION

#endif // !1666-2011

// sc_vector available since 1666-2011 (or 2.3)
#if SYSX_HAVE_SYSTEMC_ >= SYSX_MAKE_VERSION(2, 2, 90)
#define SYSX_HAVE_SC_VECTOR_
#endif

// Coware/SNPS Virtualizer?
#if defined(SYSX_HAVE_SNPS_PA) || defined(CWR_SYSTEMC)
#define SYSX_HAVE_SNPS_PA_
#endif

// include generic data analysis?
// (guess from included SystemC version)
#if defined(SYSX_HAVE_SNPS_PA_) && defined(SYSX_HAVE_SYSTEMC_2_3) &&           \
  SYSX_HAVE_SYSTEMC_2_3 >= 20120701
#define SYSX_HAVE_SNPS_DATA_ANALYSIS_
#endif

// fix sc_assert in SystemC 2.2.0
#if !defined(NDEBUG) && (SYSX_HAVE_SYSTEMC_ <= SYSX_MAKE_VERSION(2, 2, 0))

#undef sc_assert
#define sc_assert(Expr)                                                        \
  ((void)((Expr)                                                               \
            ? 0                                                                \
            : (SC_REPORT_FATAL(sc_core::SC_ID_ASSERTION_FAILED_, #Expr), 0)))

#endif // sc_assert

/* --------------------------------------------------------------------- */

// sc_core::sc_max_time()
#if !defined(IEEE_1666_SYSTEMC) || IEEE_1666_SYSTEMC < 201101L
namespace sc_core {
inline ::sc_core::sc_time const&
sc_max_time()
{
  static const ::sc_core::sc_time max_time(~SC_UINT64_ZERO, false);
  return max_time;
}
} // namespace sc_core
#endif // sc_core::sc_max_time

// sc_get_current_object()
#if SYSX_HAVE_SYSTEMC_ <= SYSX_MAKE_VERSION(2, 3, 0)
namespace sc_core {
inline ::sc_core::sc_object*
sc_get_current_object()
{
  struct dummy_object : ::sc_core::sc_object
  {
  } dummy;
  return dummy.get_parent_object();
}
} // namespace sc_core
#endif // sc_get_current_object

/* --------------------------------------------------------------------- */

#endif /* SYSX_UTILS_SYSTEMC_H_INCLUDED_ */
#endif /* SYSX_NO_SYSTEMC */
/* Taf!
 * :tag: (utils,h)
 */
