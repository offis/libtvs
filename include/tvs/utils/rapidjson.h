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
// clang-format off

#ifndef SYSX_UTILS_RAPIDJSON_H_INCLUDED_
#define SYSX_UTILS_RAPIDJSON_H_INCLUDED_

#include <tvs/utils/macros.h>

/**
 * \file rapidjson.h
 * \brief include RapidJSON library
 * \author Philipp A. Hartmann <pah@computer.org>
 *
 * \note This file is used internally within the implementation of
 *       the \ref sysx::utils::variant type, only.
 *       It is not part of the public headers.
 */

/**
 * \def SYSX_EXTERNAL_RAPIDJSON
 * \brief indication to use externally installed RapidJSON headers
 *
 * If defined, the SysX library will use the externally available
 * RapodJSON header files.
 *
 * \see SYSX_RAPIDJSON_INCLUDE_
 */

#if defined(SYSX_EXTERNAL_RAPIDJSON)
#  define SYSX_IMPL_RAPIDJSON_INCLUDE_PREFIX_ rapidjson
#else
#  define SYSX_IMPL_RAPIDJSON_INCLUDE_PREFIX_ tvs/extern/rapidjson
#endif

/**
 *
 * \brief  macro to include a RapidJSON header file
 * \param  file RapidJSON header file to include
 *
 * Unified macro to include a RapidJSON header file, either from
 * an external location or from within an embedded code copy.
 *
 * \code
 * #include SYSX_RAPIDJSON_INCLUDE_(document.h)
 * \endcode
 *
 * \hideinitializer
 */
#define SYSX_RAPIDJSON_INCLUDE_( File ) \
  SYSX_IMPL_STRINGIFY_(SYSX_IMPL_RAPIDJSON_INCLUDE_PREFIX_/File)

/* --------------------------------------------------------------- */

#ifndef SYSX_DOXYGEN_IS_RUNNING

#include <stdexcept>

// --------------------------------------------------------------------------
// configure RapidJSON

#define RAPIDJSON_NAMESPACE \
  sysx::utils::rapidjson
#define RAPIDJSON_NAMESPACE_BEGIN \
  namespace sysx { namespace utils { namespace rapidjson {
#define RAPIDJSON_NAMESPACE_END \
  } } }

// enable support for std::string
#define RAPIDJSON_HAS_STD_STRING 1

// parse floating point numbers with full precision
#define RAPIDJSON_PARSE_DEFAULT_FLAGS \
  ::RAPIDJSON_NAMESPACE::kParseFullPrecisionFlag

RAPIDJSON_NAMESPACE_BEGIN

struct AssertException
  : std::logic_error
{
  AssertException( const char* msg )
    : std::logic_error(msg) {}
};

// throw on failing assertions
#define RAPIDJSON_ASSERT( expr ) \
  ((void)(( expr ) ? 0 : \
    ( throw ::RAPIDJSON_NAMESPACE::AssertException( #expr ), 0 )))

RAPIDJSON_NAMESPACE_END

#include SYSX_RAPIDJSON_INCLUDE_(rapidjson.h)

#ifdef __GNUC__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF( pedantic ) // ignore pedantic errors
#endif

// throw exception by default
#define RAPIDJSON_PARSE_ERROR_EARLY_RETURN( what ) \
  ((void)0)
#define RAPIDJSON_PARSE_ERROR_NORETURN(parseErrorCode,offset) \
  throw ::RAPIDJSON_NAMESPACE::ParseException( \
               ::RAPIDJSON_NAMESPACE::parseErrorCode, \
               #parseErrorCode,offset \
  )

#include SYSX_RAPIDJSON_INCLUDE_(error/error.h)
#include SYSX_RAPIDJSON_INCLUDE_(error/en.h)

RAPIDJSON_NAMESPACE_BEGIN

struct ParseException
  : std::runtime_error, ParseResult
{
  ParseException( ParseErrorCode code, const char* msg, size_t offset )
     : std::runtime_error(msg), ParseResult(code,offset) {}
};

RAPIDJSON_NAMESPACE_END

#include SYSX_RAPIDJSON_INCLUDE_(document.h)
#include SYSX_RAPIDJSON_INCLUDE_(writer.h)

#ifdef __GNUC__
RAPIDJSON_DIAG_POP
#endif

#endif // SYSX_DOXYGEN_IS_RUNNING

#endif // SYSX_UTILS_RAPIDJSON_H_INCLUDED_
// :tag: (utils,s)
