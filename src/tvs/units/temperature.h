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
#ifndef SYSX_UNITS_TEMPERATURE_H_INCLUDED_
#define SYSX_UNITS_TEMPERATURE_H_INCLUDED_

#include "tvs/units/common.h"

// clang-format off
SYSX_DIAG_OFF_(unused)
#include SYSX_BOOST_INCLUDE_(units/systems/si/temperature.hpp)
SYSX_DIAG_ON_(unused)
// clang-format on

namespace sysx {
namespace units {

typedef quantity<si::temperature> temperature_type;
using si::kelvin;
using si::kelvins;

} // namespace units

namespace si {

using units::si::temperature;
using units::si::kelvins;

SYSX_UNITS_SCALED_CONSTANT_(temperature, kelvins, micro);
SYSX_UNITS_SCALED_CONSTANT_(temperature, kelvins, milli);

} // namespace si

using units::temperature_type;

} // namespace sysx

#endif // SYSX_UNITS_TEMPERATURE_H_INCLUDED_
/* Taf!
 * :tag: (units,h)
 */
