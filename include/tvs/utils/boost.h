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
// clang-format off

#ifndef SYSX_UTILS_BOOST_H_INCLUDED_
#define SYSX_UTILS_BOOST_H_INCLUDED_

#include "tvs/utils/macros.h"

#define SYSX_EXTERNAL_BOOST 1

/**
 * \def SYSX_EXTERNAL_BOOST
 * \brief indication to use externally installed Boost headers
 *
 * If defined, the SysX library will use the externally available
 * Boost header files.
 *
 * \see SYSX_BOOST_INCLUDE_
 */

#if defined(SYSX_EXTERNAL_BOOST)
#  define SYSX_IMPL_BOOST_INCLUDE_PREFIX_ boost
#else
#  define SYSX_IMPL_BOOST_INCLUDE_PREFIX_ tvs/extern/boost
#endif

/**
 *
 * \brief  macro to include a Boost header file
 * \param  file Boost header file to include
 *
 * Unified macro to include a Boost header file, either from
 * an external location or from within an embedded code copy.
 *
 * \code
 * #include SYSX_BOOST_INCLUDE_(noncopyable.hpp)
 * \endcode
 *
 * \hideinitializer
 */
#define SYSX_BOOST_INCLUDE_( File ) \
  SYSX_IMPL_STRINGIFY_(SYSX_IMPL_BOOST_INCLUDE_PREFIX_/File)

/* --------------------------------------------------------------- */

#include SYSX_BOOST_INCLUDE_(config.hpp)

#endif // SYSX_UTILS_BOOST_H_INCLUDED_
// :tag: (utils,h)
