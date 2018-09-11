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
 * \file   timed_value.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  element type for tracing streams
 * \see    timed_stream.h
 */
#ifndef TVS_TIMED_VALUE_H_INCLUDED_
#define TVS_TIMED_VALUE_H_INCLUDED_

#include <tvs/tracing/timed_duration.h>
#include <tvs/utils/variant.h>

namespace tracing {

/// type-agnostic timed value base class
class timed_value_base
{
public:
  typedef tracing::time_type time_type;
  typedef tracing::timed_duration duration_type;

  /** \name tuple duration */
  ///\{

  /// read tuple duration
  duration_type const& duration() const { return duration_; }

  /// update tuple duration
  void duration(duration_type const& d) { duration_ = d; }

  /// extend duration
  void add_duration(duration_type const& d) { duration_ += d; }

  /// infinite duration
  bool is_infinite() const { return duration_.is_infinite(); }

  /// empty duration
  bool is_delta() const { return duration_.is_delta(); }

  ///\}

protected:
  timed_value_base()
    : duration_(duration_type::infinity())
  {}

  explicit timed_value_base(const duration_type& d)
    : duration_(d)
  {}

private:
  duration_type duration_;

}; // class timed_value_base

/**
 * \brief a value with an associated "duration"
 *
 * This is the basic value class for the \c timed_stream
 * class template, which is the main interface for the
 * tracing infrastructue.
 *
 * \see timed_stream
 */
template<typename T>
struct timed_value : public timed_value_base
{
  typedef T value_type;
  typedef timed_value_base base_type;
  typedef timed_value<T> this_type;

  using base_type::duration_type;
  using base_type::time_type;

  /** \name constructors */
  ///\{

  /// default constructor - default value, infinite duration
  timed_value()
    : timed_value_base()
    , val_()
  {}

  /// value constructor - explicit value, infinite duration
  explicit timed_value(value_type const& v)
    : timed_value_base()
    , val_(v)
  {}

  /// detailed constructor - explicit value, explicit duration
  timed_value(value_type const& v, duration_type const& d)
    : timed_value_base(d)
    , val_(v)
  {}
  ///\}

  /** \name value access */
  ///\{

  void value(value_type const& v) { val_ = v; }

  value_type const& value() const { return val_; }
  value_type& value() { return val_; }

  // operator value_type const&() const { return value(); }

  ///\}

  /// print tuple to stream
  void print(std::ostream& = std::cout) const;

private:
  /// print tuple to stream
  friend std::ostream& operator<<(std::ostream& os, this_type const& tv)
  {
    tv.print(os);
    return os;
  }

  friend bool operator==(this_type const& lhs, this_type const& rhs)
  {
    return lhs.value() == rhs.value() && lhs.duration() == rhs.duration();
  }

  friend bool operator!=(this_type const& lhs, this_type const& rhs)
  {
    return !(lhs == rhs);
  }

  value_type val_;
};

/* --------------------------------------------------------------------- */

template<typename T>
void
timed_value<T>::print(std::ostream& os) const
{
  os << "(" << val_ << "," << this->duration() << ")";
}

} // namespace tracing

#endif /* TVS_TIMED_VALUE_H_INCLUDED_ */
/* Taf!
 */
