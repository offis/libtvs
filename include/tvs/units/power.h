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
#ifndef SYSX_UNITS_POWER_H_INCLUDED_
#define SYSX_UNITS_POWER_H_INCLUDED_

#include "tvs/units/common.h"

// clang-format off
SYSX_DIAG_OFF_(unused)
#include SYSX_BOOST_INCLUDE_(units/systems/si/electric_potential.hpp)
#include SYSX_BOOST_INCLUDE_(units/systems/si/resistance.hpp)
#include SYSX_BOOST_INCLUDE_(units/systems/si/conductance.hpp)
#include SYSX_BOOST_INCLUDE_(units/systems/si/capacitance.hpp)
#include SYSX_BOOST_INCLUDE_(units/systems/si/power.hpp)
#include SYSX_BOOST_INCLUDE_(units/systems/si/energy.hpp)
SYSX_DIAG_ON_(unused)
// clang-format on

namespace sysx {
namespace units {

typedef quantity<si::electric_potential> voltage_type;
using si::volts;

typedef quantity<si::capacitance> capacity_type;
using si::farads;

typedef quantity<si::resistance> resistance_type;
using si::ohms;

typedef quantity<si::conductance> conductance_type;
using si::siemens;

typedef quantity<si::energy> energy_type;
using si::joules;

typedef quantity<si::power> power_type;
using si::watts;

} // namespace units

namespace si {

using units::si::electric_potential;
typedef units::si::electric_potential voltage;
using units::si::volts;

namespace /* anonymous */ {

SYSX_UNITS_SCALED_CONSTANT_(electric_potential, volts, nano);
SYSX_UNITS_SCALED_CONSTANT_(electric_potential, volts, micro);
SYSX_UNITS_SCALED_CONSTANT_(electric_potential, volts, milli);

} // namespace anonymous

using units::si::capacitance;
using units::si::farads;

namespace /* anonymous */ {

SYSX_UNITS_SCALED_CONSTANT_(capacitance, farads, femto);
SYSX_UNITS_SCALED_CONSTANT_(capacitance, farads, pico);
SYSX_UNITS_SCALED_CONSTANT_(capacitance, farads, nano);
SYSX_UNITS_SCALED_CONSTANT_(capacitance, farads, micro);
SYSX_UNITS_SCALED_CONSTANT_(capacitance, farads, milli);

} // namespace anonymous

using units::si::ohms;
using units::si::resistance;

namespace /* anonymous */ {

SYSX_UNITS_SCALED_CONSTANT_(resistance, ohms, micro);
SYSX_UNITS_SCALED_CONSTANT_(resistance, ohms, milli);
SYSX_UNITS_SCALED_CONSTANT_(resistance, ohms, kilo);
SYSX_UNITS_SCALED_CONSTANT_(resistance, ohms, mega);

} // namespace anonymous

using units::si::conductance;
using units::si::siemens;

namespace /* anonymous */ {

SYSX_UNITS_SCALED_CONSTANT_(conductance, siemens, micro);
SYSX_UNITS_SCALED_CONSTANT_(conductance, siemens, milli);
SYSX_UNITS_SCALED_CONSTANT_(conductance, siemens, kilo);
SYSX_UNITS_SCALED_CONSTANT_(conductance, siemens, mega);

} // namespace anonymous

using units::si::energy;
using units::si::joules;

namespace /* anonymous */ {

SYSX_UNITS_SCALED_CONSTANT_(energy, joules, micro);
SYSX_UNITS_SCALED_CONSTANT_(energy, joules, milli);
SYSX_UNITS_SCALED_CONSTANT_(energy, joules, kilo);
SYSX_UNITS_SCALED_CONSTANT_(energy, joules, mega);

} // namespace anonymous

using units::si::power;
using units::si::watts;

namespace /* anonymous */ {

SYSX_UNITS_SCALED_CONSTANT_(power, watts, nano);
SYSX_UNITS_SCALED_CONSTANT_(power, watts, micro);
SYSX_UNITS_SCALED_CONSTANT_(power, watts, milli);
SYSX_UNITS_SCALED_CONSTANT_(power, watts, kilo);

} // namespace anonymous

} // namespace si

using units::capacity_type;
using units::conductance_type;
using units::energy_type;
using units::power_type;
using units::resistance_type;
using units::voltage_type;

} // namespace sysx

#endif // SYSX_UNITS_POWER_H_INCLUDED_

/* Taf!
 * :tag: (units,h)
 */
