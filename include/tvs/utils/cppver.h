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

#ifndef TVS_CPPVER_H_INCLUDED_
#define TVS_CPPVER_H_INCLUDED_

// clang-format off

// Get the active C++ standard.  Based on the SystemC 2.3.2 implementation in
// src/sysc/kernel/sc_cmnhdr.h.

#ifdef _MSC_VER
#  if defined(_MSVC_LANG) // MSVC'2015 Update 3 or later, use compiler setting
#    define TVS_CPLUSPLUS _MSVC_LANG
#  elif _MSC_VER < 1800   // MSVC'2010 and earlier, assume C++03
#    define TVS_CPLUSPLUS 199711L
#  elif _MSC_VER < 1900   // MSVC'2013, assume C++11
#    define TVS_CPLUSPLUS 201103L
#  else                   // MSVC'2015 before Update 3, assume C++14
#    define TVS_CPLUSPLUS 201402L
#  endif
#else
#  define TVS_CPLUSPLUS __cplusplus
#endif

#endif // TVS_CPPVER_H_INCLUDED_
