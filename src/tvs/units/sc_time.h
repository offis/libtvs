/*
 * Copyright (c) 2007--2010  Torsten Maehne
 * Copyright (c) 2011--2017  OFFIS Institute for Information Technology
 *                           Oldenburg, Germany
 * All rights reserved.
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
 * \file   sc_time.h
 *
 * \brief  header for the sc_core::sc_time cast function.
 *
 * This file is derived from the \c sc_time_cast.h
 * header of the \em SystemC \em AMS \em extensions \em
 * eXperiments library.
 *
 * It has been adopted to follow the namespace and naming
 * conventions of the COMPLEX simulation library.  It is usually
 * included from \ref time.h, when a SystemC context is available.
 *
 * \author    Torsten Maehne
 * \author    Philipp A. Hartmann
 *
 * \copyright Apache License, Version 2.0
 *            http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SYSX_UNITS_SC_TIME_H_INCLUDED_
#define SYSX_UNITS_SC_TIME_H_INCLUDED_

#include "tvs/units/time.h"
#include "tvs/utils/systemc.h"

namespace sysx {
namespace units {

/*!
 * \brief  Casts sc_core::sc_time to other scalar types, which
 *         represent the time in SI seconds.
 *
 * \tparam T The target scalar type for the sc_core::sc_time cast
 * \param  t The sc_core::sc_time value.
 * \return   Converted value of t assuming SI seconds as the unit.
 *
 * \todo   Check for a better name for sc_time_cast<T>.
 */
template<typename T>
T
sc_time_cast(const sc_core::sc_time& t);

template<>
inline sc_core::sc_time
sc_time_cast(const sc_core::sc_time& t)
{
  return t;
}

template<>
inline float
sc_time_cast(const sc_core::sc_time& t)
{
  return t.to_seconds();
}

template<>
inline double
sc_time_cast(const sc_core::sc_time& t)
{
  return t.to_seconds();
}

template<>
inline long double
sc_time_cast(const sc_core::sc_time& t)
{
  return t.to_seconds();
}

template<>
inline quantity<si::time>
sc_time_cast(const sc_core::sc_time& t)
{
  return t.to_seconds() * si::second;
}

/*!
 * \brief  Lazily evaluated function wrapper for sc_time_cast to
 *         facilitate its use in functors created with the
 *         Boost.Lambda library.
 *
 * \tparam T The target scalar type for the sc_core::sc_time cast.
 */
template<typename T>
struct sc_time_cast_type
{
  typedef T result_type; //!< Result type of \ref sc_time_cast<T>

  //! Pass call to scax_util::sc_time_cast<T> function.
  T operator()(const sc_core::sc_time& t) const { return sc_time_cast<T>(t); }
};

/*!
 * \brief  Casts a scalar value, which represent the time in SI seconds,
 *         to sc_core::sc_time.
 */
template<typename T>
sc_core::sc_time
sc_time_cast(const T& t);

namespace impl {

/*!
 * \brief Implements the conversion of a scalar value, which represents the
 *        time in seconds, to sc_core::sc_time.
 *
 * \note  This class is a pure implementation artefact, as templatized
 *        functions cannot be partially specialized, which is necessary for
 *        the special treatment of quantity types.
 */
template<typename T>
struct sc_time_cast_impl;

template<>
struct sc_time_cast_impl<float>
{
  static sc_core::sc_time apply(const float& t)
  {
    return sc_core::sc_time(t, sc_core::SC_SEC);
  }
};

template<>
struct sc_time_cast_impl<double>
{
  static sc_core::sc_time apply(const double& t)
  {
    return sc_core::sc_time(t, sc_core::SC_SEC);
  }
};

template<>
struct sc_time_cast_impl<long double>
{
  static sc_core::sc_time apply(const long double& t)
  {
    return sc_core::sc_time(t, sc_core::SC_SEC);
  }
};

template<typename T>
struct sc_time_cast_impl<quantity<si::time, T>>
{
  static sc_core::sc_time apply(const quantity<si::time, T>& t)
  {
    return sc_core::sc_time(t.value(), sc_core::SC_SEC);
  }
};

template<>
struct value_helper<sc_core::sc_time>
{
  typedef sc_core::sc_time type;
  typedef sc_core::sc_time value_type;

  static type max()
  {
    return sc_core::sc_max_time(); /** \todo add support for 2.2.0 */
  }

  static type infinity() { return max(); }

  static bool is_infinite(type const& v) { return v == infinity(); }
};

} // namespace impl

template<typename T>
inline sc_core::sc_time
sc_time_cast(const T& t)
{
  return impl::sc_time_cast_impl<T>::apply(t);
}

} // namespace units
} // namespace sysx

#endif // SYSX_UNITS_SC_TIME_H_INCLUDED_

/* Taf!
 * :tag: (units,h)
 */
