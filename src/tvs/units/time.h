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
#ifndef SYSX_UNITS_TIME_H_INCLUDED_
#define SYSX_UNITS_TIME_H_INCLUDED_

#include "tvs/units/common.h"

// clang-format off
SYSX_DIAG_OFF_(unused)
#include SYSX_BOOST_INCLUDE_(units/systems/si/time.hpp)
#include SYSX_BOOST_INCLUDE_(units/systems/si/frequency.hpp)
#include SYSX_BOOST_INCLUDE_(units/base_units/si/second.hpp)
SYSX_DIAG_ON_(unused)
// clang-format on

namespace sysx {
namespace units {

typedef quantity<si::time> time_type;
using si::seconds;

typedef quantity<si::frequency> frequency_type;
using si::hertz;

typedef quantity<si::dimensionless, unsigned int> cycle_count_type;

time_type
max_time();

bool
is_infinite(time_type const& time);

} // namespace units

namespace si {

using units::si::time;
using units::si::seconds;

namespace /* anonymous */ {

SYSX_UNITS_SCALED_CONSTANT_(time, seconds, pico);
SYSX_UNITS_SCALED_CONSTANT_(time, seconds, nano);
SYSX_UNITS_SCALED_CONSTANT_(time, seconds, micro);
SYSX_UNITS_SCALED_CONSTANT_(time, seconds, milli);

} /* namespace anonymous */

using units::si::frequency;
using units::si::hertz;

namespace /* anonymous */ {

SYSX_UNITS_SCALED_CONSTANT_(frequency, hertz, kilo);
SYSX_UNITS_SCALED_CONSTANT_(frequency, hertz, mega);
SYSX_UNITS_SCALED_CONSTANT_(frequency, hertz, giga);

} /* namespace anonymous */

} // namespace si

using units::time_type;
using units::frequency_type;
using units::cycle_count_type;

} // namespace sysx

#endif // SYSX_UNITS_TIME_H_INCLUDED_

// include compatiblity with sc_time, iff SystemC is available
// (deliberately outside of include guards!)
#if defined(SYSX_HAVE_SYSTEMC_)
#include "tvs/units/sc_time.h"
#endif // SYSX_SYSTEMC_INCLUDED_

/* Taf!
 * :tag: (units,h)
 */
