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

#ifndef SYSX_UTILS_NONCOPYABLE_H_INCLUDED_
#define SYSX_UTILS_NONCOPYABLE_H_INCLUDED_

/**
 * \file   noncopyable.h
 * \brief  suppress (implicit) object copying
 * \author Philipp A. Hartmann <pah@computer.org>
 */

#ifdef HAVE_BOOST

#include <tvs/utils/boost.h>
#include SYSX_BOOST_INCLUDE_(noncopyable.hpp)

#else // no Boost - use simplified implementation

namespace sysx_adl_barrier_ {

class noncopyable
{
protected:
  noncopyable() = default;
  ~noncopyable() = default;

private: // emphasize the following members are private
  noncopyable(const noncopyable&) = delete;
  const noncopyable& operator=(const noncopyable&) = delete;
};

} /* namespace sysx_adl_barrier_ */

#endif /* HAVE_BOOST */

namespace sysx {
namespace utils {

/**
 * \brief base class to ensure that a class can not be copied
 *
 * Inheriting from this class ensures, that the derived
 * class(es) can not be copied through copy-constructor
 * or assignment operator.
 *
 * Explicit or implicit attempts to copy an instance
 * of derived classes result in a compilation error.
 *
 * Example:
 * \code
 * class my_class
 *   : public  my_base_class // other base classes
 *   , private ::sysx::utils::noncopyable
 * {
 *   // class body ...
 * };
 * \endcode
 *
 * \note This implementation is based on
 *       \c boost::noncopyable, see http://www.boost.org
 */
#ifdef HAVE_BOOST
typedef ::boost::noncopyable noncopyable;
#else
typedef ::sysx_adl_barrier_::noncopyable noncopyable;
#endif /* HAVE_BOOST */

} /* namespace utils */
} /* namespace sysx */

#endif /* SYSX_UTILS_NONCOPYABLE_H_INCLUDED_ */
/* Taf!
 * :tag: (utils,h)
 */
