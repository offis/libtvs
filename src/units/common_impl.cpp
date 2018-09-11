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
#include <map>
#include <sstream>

#include "tvs/utils/variant_traits.h"

#include "tvs/utils/assert.h"
#include "tvs/utils/report.h"

#include "tvs/units/common.h"
#include "tvs/units/report_msgs.h"

#include "tvs/units/power.h"
#include "tvs/units/temperature.h"
#include "tvs/units/time.h"

#include "tvs/units/common_impl.tpp"

#ifndef SYSX_NO_SYSTEMC
#include "tvs/units/sc_time.h"
#endif

#include <limits>

namespace sysx {
namespace units {

/// extract scaling factor from unit prefix
quantity<si::dimensionless>
from_prefix(std::string const& prefix)
{
  typedef std::map<std::string, quantity<si::dimensionless>> table_type;
  static table_type prefix_table;

  // no prefix given
  if (!prefix.size())
    return 1. * si::si_dimensionless;

  if (sysx_unlikely(!prefix_table.size())) {
#define REGISTER_PREFIX(Arg)                                                   \
  prefix_table.insert(make_pair(                                               \
    to_string(Arg), static_cast<quantity<si::dimensionless>>(1. * Arg)))

    REGISTER_PREFIX(si::femto);
    REGISTER_PREFIX(si::pico);
    REGISTER_PREFIX(si::nano);
    REGISTER_PREFIX(si::micro);
    REGISTER_PREFIX(si::milli);
    REGISTER_PREFIX(si::kilo);
    REGISTER_PREFIX(si::mega);
    REGISTER_PREFIX(si::giga);

#undef REGISTER_PREFIX
  }

  // lookup scaling factor in prefix table
  table_type::const_iterator pit = prefix_table.find(prefix);

  if (pit == prefix_table.end())
    SYSX_REPORT_ERROR(report::invalid_prefix_symbol) % prefix;

  return pit->second;
}

#define REGISTER_UNIT(Unit, Init)                                              \
  namespace impl {                                                             \
  template<>                                                                   \
  /*typename*/ parse_quantity<si::Unit>::quantity_type const                   \
    parse_quantity<si::Unit>::one = 1. * si::Init;                             \
  template<>                                                                   \
  std::string const parse_quantity<si::Unit>::unit_name =                      \
    SYSX_IMPL_STRINGIFY_(Unit);                                                \
  }                                                                            \
  template quantity<si::Unit, double> from_string(std::string const&);         \
  template quantity<si::Unit, double> from_symbol(std::string const&)

REGISTER_UNIT(time, seconds);
REGISTER_UNIT(frequency, hertz);

REGISTER_UNIT(electric_potential, volts);
REGISTER_UNIT(capacitance, farads);
REGISTER_UNIT(resistance, ohms);
REGISTER_UNIT(conductance, siemens);
REGISTER_UNIT(power, watts);
REGISTER_UNIT(energy, joules);

REGISTER_UNIT(temperature, kelvins);

#define INSTANTIATE_LIMITS(Type)                                               \
  template Type infinity<Type>();                                              \
  template Type max_value<Type>();                                             \
  template bool is_infinite<Type>(Type const&)

INSTANTIATE_LIMITS(time_type);
#ifndef SYSX_NO_SYSTEMC
INSTANTIATE_LIMITS(sc_core::sc_time);
#endif

} // namespace units
} // namespace sysx

namespace sysx {
namespace utils {

template struct variant_traits<units::quantity<si::time>>;
template struct variant_traits<units::quantity<si::electric_potential>>;
template struct variant_traits<units::quantity<si::capacitance>>;
template struct variant_traits<units::quantity<si::resistance>>;
template struct variant_traits<units::quantity<si::conductance>>;
template struct variant_traits<units::quantity<si::power>>;
template struct variant_traits<units::quantity<si::energy>>;
template struct variant_traits<units::quantity<si::temperature>>;

} /* namespace utils */
} /* namespace sysx */

/* Taf!
 * :tag: (units,s)
 */
