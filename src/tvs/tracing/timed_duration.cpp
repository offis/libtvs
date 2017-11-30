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
 * \file   timed_duration.cpp
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  duration type for tracing streams (implementation)
 * \see    timed_stream.h
 */

#include "tvs/tracing/timed_duration.h"

#include <iostream>
#include <limits>

#ifdef max // we're using std::numeric_limits<>::max
#undef max
#endif

namespace tracing {

const timed_duration::value_type timed_duration::zero_time;

/* --------------------------------------------------------------------- */

timed_duration&
timed_duration::operator+=(timed_duration const& that)
{
  if (sysx_likely(!is_infinite())) {
    if (sysx_unlikely(sysx::units::max_value<value_type>() - val_ <=
                      that.val_)) {
      /// \todo add warning?
      val_ = sysx::units::infinity<value_type>();
    } else {
      val_ += that.val_;
    }
  }
  return *this;
}

timed_duration&
timed_duration::operator-=(timed_duration const& that)
{
  if (sysx_likely(!is_infinite())) {
    if (sysx_unlikely(val_ < that.val_)) {
      /// \todo add warning?
      val_ = zero_time;
    } else {
      val_ -= that.val_;
    }
  }
  return *this;
}

#if !defined(SYSX_NO_SYSTEMC)
timed_duration&
timed_duration::operator%=(timed_duration const& that)
{
  val_ %= that.val_;
  return *this;
}
#endif

/* --------------------------------------------------------------------- */

timed_duration&
timed_duration::operator*=(double s)
{
  /// \todo add overflow/underflow check
  val_ *= s;
  return *this;
}

timed_duration&
timed_duration::operator/=(double s)
{
  /// \todo add overflow/underflow check
  val_ /= s;
  return *this;
}

/* --------------------------------------------------------------------- */

void
timed_duration::print(std::ostream& os) const
{
  if (is_infinite())
    os << "inf";
  else
    os << val_;
}

} // namespace tracing
/* Taf!
 */
