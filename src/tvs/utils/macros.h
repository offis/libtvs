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

#ifndef SYSX_UTILS_MACROS_H_INCLUDED_
#define SYSX_UTILS_MACROS_H_INCLUDED_
/**
 * \file macros.h
 *
 * \brief common macro helpers
 *
 */

/* ------------------------ library version ----------------------- */

/**
 * \brief small helper macro for version comparisons
 *
 * returns a (potentially preprocessor checkable) constant
 * for a given version triple (numbers only)
 *
 * \note Each part of the version is has to be smaller
 *       than 256, otherwise the returned constant is no longer
 *       unique.
 */
#define SYSX_MAKE_VERSION(x, y, z) (((x) << 16) + ((y) << 8) + (z))

/* --------------------------- platform --------------------------- */

/* Linux */
#if defined(__linux__)
#define SYSX_LINUX_ __linux__

/* CygWin */
#elif defined(__CYGWIN__) || defined(__CYGWIN32)
#define SYSX_CYGWIN_ 1

/* 32-bit Windows */
#elif defined(_WIN32)
#define SYSX_WIN32_ _WIN32
#elif defined(WIN32)
#define SYSX_WIN32_ WIN32

#endif /* platform selection */

/* --------------------------- compiler --------------------------- */

/* Intel's compiler */
#if defined(__INTEL_COMPILER)
#define SYSX_ICC_ __INTEL_COMPILER
#elif defined(__ICC)
#define SYSX_ICC_ __ICC
#endif

/* MinGW */
#if defined(__MINGW32__)
#define SYSX_MINGW_ __MINGW32__
#if !defined(SYSX_WIN32_)
#define SYSX_WIN32_ SYSX_MINGW_
#endif
#endif

/* Clang */
#if defined(__clang__)
#define SYSX_CLANG_ __clang_version__
#endif

/* GCC C++ compiler (gets set by Intel compiler and Clang as well) */
#if defined(__GNUC__)
#define SYSX_GCC_                                                              \
  SYSX_MAKE_VERSION(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#endif

/* Microsoft Visual C++ compiler */
#if defined(_MSC_VER)
#define SYSX_MSC_ _MSC_VER
#elif defined(MSC_VER)
#define SYSX_MSC_ MSC_VER
#endif
#if defined(SYSX_MSC_)
#if (SYSX_MSC_ >= 1310) /* Visual C++ 7.0, resp. .NET 2003 */
#define SYSX_MSC_NET_ SYSX_MSC_
#else
#error Microsoft Visual C++ prior to version 7.1 (.NET 2003) is \
         unsupported. Please upgrade or choose another platform.
#endif
#endif

/* ---------------------- compiler warnings ----------------------- */

#if defined(SYSX_CLANG_) || (SYSX_GCC_ >= SYSX_MAKE_VERSION(4, 2, 0))

#define SYSX_IMPL_DIAG_DO_PRAGMA_(x) _Pragma(SYSX_IMPL_STRINGIFY_(x))
#define SYSX_IMPL_DIAG_PRAGMA_(x) SYSX_IMPL_DIAG_DO_PRAGMA_(GCC diagnostic x)

#if defined(SYSX_CLANG_) || (SYSX_GCC_ >= SYSX_MAKE_VERSION(4, 6, 0))

#define SYSX_DIAG_OFF_(x)                                                      \
  SYSX_IMPL_DIAG_PRAGMA_(push)                                                 \
  SYSX_IMPL_DIAG_PRAGMA_(ignored SYSX_IMPL_STRINGIFY_(SYSX_IMPL_CONCAT_(-W, x)))

#define SYSX_DIAG_ON_(x) SYSX_IMPL_DIAG_PRAGMA_(pop)

#else // SYSX_GCC_ >= 4.2.0

#define SYSX_DIAG_OFF_(x)                                                      \
  SYSX_IMPL_DIAG_PRAGMA_(ignored SYSX_IMPL_STRINGIFY_(SYSX_IMPL_CONCAT_(-W, x)))

#define SYSX_DIAG_ON_(x)                                                       \
  SYSX_IMPL_DIAG_PRAGMA_(warning SYSX_IMPL_STRINGIFY_(SYSX_IMPL_CONCAT_(-W, x)))

#endif // GCC 4.2.0

// TODO: add MSVC support
// #elif defined( SYSX_MSC_ )

#else // -- ignore compiler warning directives

/**
 * \def SYSX_DIAG_OFF_
 * \brief hide compiler warning
 * \param What compiler warning to hide (compiler-specific)
 *
 * Disables a compiler warning for the following section of
 * the code.  The parameter is a compiler-specific identifier
 * for the warning.
 *
 * \note Currently, only GCC (compatible) warning options are
 *       supported.
 *
 * Macro expands to nothing, if no supported compiler (version)
 * is detected.
 *
 * \hideinitializer
 */
#define SYSX_DIAG_OFF_(What)

/**
 * \def SYSX_DIAG_ON_
 * \brief restore compiler warning
 * \param What compiler warning to restore (compiler-specific)
 *
 * Disables a compiler warning for the following section of
 * the code.  The parameter is a compiler-specific identifier
 * for the warning.
 *
 * \note Currently, only GCC (compatible) warning options are
 *       supported.
 *
 * Macro expands to nothing, if no supported compiler (version)
 * is detected.
 *
 * \hideinitializer
 */
#define SYSX_DIAG_ON_(What)

#endif

/* ---------------------------- debug ----------------------------- */

/* default: debug build */
#if !defined(DEBUG) && !defined(NDEBUG)
#define NDEBUG
#endif

/* build debug version */
#ifdef DEBUG
/* enable minimum debugging, if not requested explicitly */
#ifndef SYSX_DEBUGLEVEL
#define SYSX_DEBUGLEVEL 1
#endif
#endif /* DEBUG */

/* build optimized version */
#ifdef NDEBUG
/* disable debugging, if not requested explicitly */
#ifndef SYSX_DEBUGLEVEL
#define SYSX_DEBUGLEVEL 0
#endif
#endif /* NDEBUG */

/* ---------------------- branch prediction ----------------------- */

#if !defined(SYSX_GCC_) || (SYSX_GCC_ < SYSX_MAKE_VERSION(2, 96, 0))
// no branch prediction available here

/**
 * \def sysx_likely()
 * \brief branch prediction helper
 *
 * This macro can be used to give a hint to some compilers, that
 * the truth value of the given expression is more likely to
 * be \c true. The compiler can than generate appropriate
 * branch-prediction friendly code.
 *
 * \note Currently, this has no effect on compilers other than
 *       GNU C++ Compiler >= 2.96.
 *
 * \param   x Expression to be checked
 * \return  truth value of x
 *
 * This technique was inspired by the Linux kernel macros
 * likely() and unlikely().
 *
 * \see SYSX_unlikely(),
 *      http://www.geocities.com/kedarsovani/pubs/likely_unlikely.html
 */
#define sysx_likely(x) !!(x)
/**
 * \def sysx_unlikely()
 * \brief branch prediction helper
 *
 * This macro can be used to give a hint to some compilers, that
 * the truth value of the given expression is more likely to
 * be \c false. The compiler can than generate appropriate
 * branch-prediction friendly code.
 *
 * \note Currently, this has no effect on compilers other than
 *       GNU C++ Compiler >= 2.96.
 *
 * \param   x Expression to be checked
 * \return  truth value of x
 *
 * This technique was inspired by the Linux kernel macros
 * likely() and unlikely().
 *
 * \see sysx_likely(),
 *      http://www.geocities.com/kedarsovani/pubs/likely_unlikely.html
 */
#define sysx_unlikely(x) !!(x)
#else
// use the optimized expressions
#define sysx_likely(x) __builtin_expect(!!(x), 1)
#define sysx_unlikely(x) __builtin_expect(!!(x), 0)
#endif

/* ---------------------- token concatenation --------------------- */

/**
 * \def SYSX_IMPL_CONCAT_( First, Second )
 * \brief concatenates two arguments
 *
 * Internal macro to concatenate two preprocessor tokens.
 *
 * \return arguments joined to a single token
 * \hideinitializer
 */
#define SYSX_IMPL_CONCAT_(First, Second)                                       \
  SYSX_IMPL_CONCAT_DEFERRED_(First, Second)

/**
 * \def SYSX_IMPL_CONCAT_DEFERRED_( First, Second )
 * \brief (first) concatenation helper
 *
 * Internal macro, do not use directly.
 *
 * \see SYSX_IMPL_CONCAT_( First, Second )
 * \hideinitializer
 */
#define SYSX_IMPL_CONCAT_DEFERRED_(First, Second)                              \
  SYSX_IMPL_CONCAT_EVEN_MORE_DEFERRED_(First, Second)

/**
 * \def SYSX_IMPL_CONCAT_EVEN_MORE_DEFERRED_( First, Second )
 * \brief (second) concatenation helper
 *
 * Internal macro, do not use directly.
 *
 * \see SYSX_IMPL_CONCAT_( First, Second )
 * \hideinitializer
 */
#define SYSX_IMPL_CONCAT_EVEN_MORE_DEFERRED_(First, Second) First##Second

/* --------------------- token stringification -------------------- */

/**
 * \def SYSX_IMPL_STRINGIFY_( Arg )
 * \brief convert argument to string constant
 *
 * Internal macro to stringify preprocessor tokens.
 *
 * \param Arg to stringify
 * \hideinitializer
 *
 */
#define SYSX_IMPL_STRINGIFY_(Arg) SYSX_IMPL_STRINGIFY_DEFERRED_(Arg)

/**
 * \def SYSX_IMPL_STRINGIFY_DEFERRED_( Arg )
 * \brief stringification helper
 *
 * Internal macro, do not use directly.
 *
 * \see SYSX_IMPL_STRINGIFY_( Arg )
 * \hideinitializer
 */
#define SYSX_IMPL_STRINGIFY_DEFERRED_(Arg) #Arg

/* ---------------------- unused variables ------------------------ */

#if defined(SYSX_GCC_)

#define SYSX_IMPL_UNUSED_(x) x __attribute__((__unused__))

#else

/**
 * \def SYSX_IMPL_UNUSED_( x )
 * \brief   indicate an unused variable
 *
 * \param x identifier to be marked as unused
 * \hideinitializer
 */
#define SYSX_IMPL_UNUSED_(x) x

#endif // SYSX_IMPL_UNUSED_

#define SYSX_IMPL_NOTHING_ /* nothing */

/**
 * \def SYSX_ANONYMOUS_VARIABLE( x )
 * \brief   create an anonymous variable name
 *
 * \param x variable prefix
 * \hideinitializer
 */
#define SYSX_ANONYMOUS_VARIABLE(seed) SYSX_IMPL_CONCAT_(seed, __LINE__)

/* ---------------------------- misc ------------------------------ */

#if defined(SYSX_MSC_)

/* In Visual C++ __PRETTY_FUNCTION__ and __func__ are not available,
   so we use __FUNCTION__ here instead */
#define __func__ __FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCTION__

/* C++ exception specification ignored except to indicate
 * a function is not __declspec(nothrow) */
#pragma warning(disable : 4290)

#endif /* SYSX_MSC_ */

/**
 * \namespace sysx
 * \brief main SysX library namespace
 *
 * This is the public namespace of the SysX SystemC eXtensions library.
 * All elements of this library are encaspulated within this namespace.
 *
 * The only exception are source files from external vendors, like Boost,
 * or Accellera (of course).  In this case, the upstream classes remain in
 * their original namespace (provided they where shipped with such a
 * namespace).
 *
 * As a rule of thumb, the elements of the \c sysx namespace can be
 * considered as the public API of the SysX simulation library.
 * Implementation-specific elements are usually moved to the
 * nested namespace \c sysx::impl.
 *
 * \warning Since this library is currently under development, please
 *          do not rely on the contents of this namespace.
 *          For explicitly public elements, please refer to the
 *          exported symbols from within sysx.h.
 *
 * \see sysx.h
 */
namespace sysx { /* here for documentation only */

/**
 * \namespace sysx::impl
 * \brief internal SysX library namespace
 *
 * This namespace contains implementation-specific elements of
 * the SystemC eXtension library.  Usually, you should not need
 * to bother with its contents.
 *
 * \warning Do not use any element of this namespace in any SysX model
 *          directly, since new versions of the SysX library may change
 *          any element within this namespace at any time.
 */
namespace impl { /* here for documentation only */
}

} /* namespace sysx */

#endif /* SYSX_UTILS_MACROS_H_INCLUDED_ */
/* Taf!
 * :tag: (utils,h)
 */
