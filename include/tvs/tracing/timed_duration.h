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

/**
 * \file   timed_duration.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  duration type for tracing streams
 * \see    timed_stream.h
 */
#ifndef TVS_TIMED_DURATION_H_INCLUDED_
#define TVS_TIMED_DURATION_H_INCLUDED_

#include <tvs/units/time.h>
#include <tvs/utils/type_id.h>

#include <iosfwd>
#include <iostream> // std::cout

namespace tracing {

#ifndef SYSX_NO_SYSTEMC
typedef sc_core::sc_time time_type;
#else
typedef sysx::units::time_type time_type;
#endif

class timed_duration;

typedef timed_duration duration_type;

/**
 * \brief duration handling
 *
 * The timed_duration class provides an extended interface compared
 * to the regular SystemC \c sc_time class or the Boost.Units
 * time implementation to represent durations.
 *
 * Most importantly, an infinite value is supported and the
 * overflow handling is added to saturate at infinity.
 *
 * \see sc_core::sc_time, sysx::units::time_type
 */
class timed_duration
{
public:
  typedef timed_duration this_type;

  // native time type for the streams
  typedef time_type value_type;

  // boost::units time type
  typedef sysx::units::time_type units_type;

  timed_duration()
    : val_()
  {}

  timed_duration(value_type const& vd)
    : val_(vd)
  {}

#ifndef SYSX_NO_SYSTEMC
  explicit timed_duration(units_type const& ud)
    : val_(sysx::units::sc_time_cast<units_type>(ud))
  {}

  operator units_type() const
  {
    return sysx::units::sc_time_cast<units_type>(value());
  }
#endif

  operator value_type () const { return value(); }

  value_type const& value() const { return val_; }

  void swap(this_type& that)
  {
    value_type tmp = val_;
    val_ = that.val_;
    that.val_ = tmp;
  }

  /** \brief special values */
  ///\{
  bool is_infinite() const
  {
    return sysx::units::is_infinite<value_type>(val_);
  }

  bool is_delta() const { return val_ == value_type(); }

  static this_type const max_time()
  {
    return this_type(sysx::units::max_value<value_type>());
  }

  static this_type const infinity()
  {
    return this_type(sysx::units::infinity<value_type>());
  }

  static value_type const zero_time;
  ///\}

  /**\name arithmetic operators */
  ///\{
  this_type& operator+=(this_type const&);
  this_type& operator-=(this_type const&);
  this_type& operator*=(double);
  this_type& operator/=(double);

#if !defined(SYSX_NO_SYSTEMC)
  this_type& operator%=(this_type const&);
#endif
  ///\}

  /**\name relational operators */
  ///\{
  friend bool operator<(timed_duration const& d1, timed_duration const& d2)
  {
    return d1.val_ < d2.val_; /** \todo handle infinity */
  }
  friend bool operator>(timed_duration const& d1, timed_duration const& d2)
  {
    return d2 < d1;
  }
  friend bool operator<=(timed_duration const& d1, timed_duration const& d2)
  {
    return !(d1 > d2);
  }
  friend bool operator>=(timed_duration const& d1, timed_duration const& d2)
  {
    return !(d1 < d2);
  }
  friend bool operator==(timed_duration const& d1, timed_duration const& d2)
  {
    return d1.val_ == d2.val_; /** \todo handle infinity */
  }
  friend bool operator!=(timed_duration const& d1, timed_duration const& d2)
  {
    return !(d1 == d2);
  }
  ///\}

  /// print duration to stream
  void print(std::ostream& = std::cout) const;

  /// print duration to stream
  friend std::ostream& operator<<(std::ostream& os, timed_duration const& td)
  {
    td.print(os);
    return os;
  }

private:
  static bool check_infinity(value_type const&);
  value_type val_;
};

#define SYSX_TIMED_DURATION_BINOP_OTHER_(Op, OtherType)                        \
  inline timed_duration operator Op(OtherType const& d1,                       \
                                    timed_duration const& d2)                  \
  {                                                                            \
    return timed_duration(d1) Op## = d2;                                       \
  }                                                                            \
  inline timed_duration operator Op(timed_duration d1, const OtherType& d2)    \
  {                                                                            \
    return d1 Op## = timed_duration(d2);                                       \
  }

#ifndef SYSX_NO_SYSTEMC
#define SYSX_TIMED_DURATION_BINOP_UNITS_(Op)                                   \
  SYSX_TIMED_DURATION_BINOP_OTHER_(Op, timed_duration::units_type)
#else
#define SYSX_TIMED_DURATION_BINOP_UNITS_(Op) /* empty */
#endif

#define SYSX_TIMED_DURATION_BINOP_(Op)                                         \
  inline timed_duration operator Op(timed_duration d1,                         \
                                    timed_duration const& d2)                  \
  {                                                                            \
    return d1 Op## = d2;                                                       \
  }                                                                            \
  SYSX_TIMED_DURATION_BINOP_UNITS_(Op)                                         \
  SYSX_TIMED_DURATION_BINOP_OTHER_(Op, timed_duration::value_type)

SYSX_TIMED_DURATION_BINOP_(+)
SYSX_TIMED_DURATION_BINOP_(-)

#ifndef SYSX_NO_SYSTEMC
SYSX_TIMED_DURATION_BINOP_(%)
#endif

#define SYSX_TIMED_DURATION_RELOP_OTHER_(Op, OtherType)                        \
  inline bool operator Op(OtherType const& d1, timed_duration const& d2)       \
  {                                                                            \
    return timed_duration(d1) Op d2;                                           \
  }                                                                            \
  inline bool operator Op(timed_duration d1, const OtherType& d2)              \
  {                                                                            \
    return d1 Op timed_duration(d2);                                           \
  }

#ifndef SYSX_NO_SYSTEMC
#define SYSX_TIMED_DURATION_RELOP_UNITS_(Op)                                   \
  SYSX_TIMED_DURATION_RELOP_OTHER_(Op, timed_duration::units_type)
#else
#define SYSX_TIMED_DURATION_RELOP_UNITS_(Op) /* empty */
#endif

#define SYSX_TIMED_DURATION_RELOP_(Op)                                         \
  SYSX_TIMED_DURATION_RELOP_UNITS_(Op)                                         \
  SYSX_TIMED_DURATION_RELOP_OTHER_(Op, time_type)

SYSX_TIMED_DURATION_RELOP_(<)
SYSX_TIMED_DURATION_RELOP_(>)
SYSX_TIMED_DURATION_RELOP_(<=)
SYSX_TIMED_DURATION_RELOP_(>=)
SYSX_TIMED_DURATION_RELOP_(==)
SYSX_TIMED_DURATION_RELOP_(!=)

#define SYSX_TIMED_DURATION_SCALAR_OP_(Op)                                     \
  inline timed_duration operator Op(double s, timed_duration d)                \
  {                                                                            \
    return d Op## = s;                                                         \
  }                                                                            \
  inline timed_duration operator Op(timed_duration d, double s)                \
  {                                                                            \
    return d Op## = s;                                                         \
  }

SYSX_TIMED_DURATION_SCALAR_OP_(*)
SYSX_TIMED_DURATION_SCALAR_OP_(/)

/* --------------------------------------------------------------------- */

#undef SYSX_TIMED_DURATION_BINOP_
#undef SYSX_TIMED_DURATION_BINOP_UNITS_
#undef SYSX_TIMED_DURATION_BINOP_OTHER_
#undef SYSX_TIMED_DURATION_RELOP_
#undef SYSX_TIMED_DURATION_RELOP_UNITS_
#undef SYSX_TIMED_DURATION_RELOP_OTHER_
#undef SYSX_TIMED_DURATION_SCALAR_OP_

} // namespace tracing


namespace sysx {
namespace utils {

#define VARIANT_TRAITS_DERIVED_(UnderlyingType, SpecializedType)  \
  template<>                                                      \
  struct variant_traits<SpecializedType>                          \
    : variant_traits_convert<SpecializedType, UnderlyingType>     \
  {                                                               \
  }

// timed_duration derived from underlying time type
VARIANT_TRAITS_DERIVED_(tracing::time_type, tracing::timed_duration);

#undef VARIANT_TRAITS_DERIVED_

} /* namespace utils */
} /* namespace sysx */

#endif /* TVS_TIMED_DURATION_H_INCLUDED_ */
/* Taf!
 */
