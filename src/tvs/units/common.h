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
#ifndef SYSX_UNITS_COMMON_H_INCLUDED_
#define SYSX_UNITS_COMMON_H_INCLUDED_

#include "tvs/utils/boost.h"
#include "tvs/utils/variant_traits.h"

// clang-format off
// GCC & Clang warn about unused typedefs and constants in Boost
SYSX_DIAG_OFF_(unused)
SYSX_DIAG_OFF_(unused-parameter)
#if defined(SYSX_GCC_) && SYSX_GCC_ >= SYSX_MAKE_VERSION(4,8,0)
SYSX_DIAG_OFF_(unused-local-typedefs)
#endif

#include SYSX_BOOST_INCLUDE_(units/units_fwd.hpp)
#include SYSX_BOOST_INCLUDE_(units/quantity.hpp)
#include SYSX_BOOST_INCLUDE_(units/scale.hpp)
#include SYSX_BOOST_INCLUDE_(units/io.hpp)
#include SYSX_BOOST_INCLUDE_(units/systems/si/io.hpp)
#include SYSX_BOOST_INCLUDE_(units/systems/si/prefixes.hpp)

SYSX_DIAG_ON_(unused)
SYSX_DIAG_ON_(unused-parameter)
#if defined(SYSX_GCC_) && SYSX_GCC_ >= SYSX_MAKE_VERSION(4,8,0)
SYSX_DIAG_ON_(unused-local-typedefs)
#endif
// clang-format on

namespace boost {
namespace units {

#ifdef SYSX_DOXYGEN_IS_RUNNING

/// quantity for Units type
template<typename Unit, typename ValueType = double>
class quantity
{
};

/// SI unit system
namespace si {

} // namespace si

#endif // Doxygen forward declarations

/// symbol string convenience overload for quantities
template<typename Unit, typename ValueType>
std::string
symbol_string(quantity<Unit, ValueType> const&)
{
  return symbol_string(Unit());
}

} // namespace units
} // namespace boost

namespace sysx {
namespace units {

using boost::units::quantity;
namespace si = boost::units::si;

/// extract value and unit type from string
template<typename Unit /*, typename ValueType*/>
quantity<Unit /*,ValueType*/>
from_string(std::string const&);

/// extract unit type (and scaling factor) from unit symbol
template<typename Unit /*, typename ValueType*/>
quantity<Unit /*,ValueType*/>
from_symbol(std::string const&);

/// extract scaling factor from unit prefix
quantity<si::dimensionless>
from_prefix(std::string const&);

using boost::units::engineering_prefix;

namespace impl {

template<typename T>
struct value_helper;

} // namespace impl

template<typename T>
bool
is_infinite(T const& v);

template<typename T>
T
infinity();

template<typename T>
T
max_value();

} // namespace units

namespace si {

using units::si::femto;
using units::si::giga;
using units::si::kilo;
using units::si::mega;
using units::si::micro;
using units::si::milli;
using units::si::nano;
using units::si::pico;

} // namespace si

} // namespace sysx

#define SYSX_UNITS_SCALED_CONSTANT_(Unit, Name, Scale)                         \
  static const ::sysx::units::quantity<Unit> SYSX_IMPL_UNUSED_(                \
    SYSX_IMPL_CONCAT_(Scale, Name))(1.0 * Scale * Name)

namespace sysx {
namespace utils {

/// unit<->variant conversion
template<typename Unit, typename ValueType>
struct variant_traits<::sysx::units::quantity<Unit, ValueType>>
{
  typedef ::sysx::units::quantity<Unit, ValueType> type;
  static bool pack(variant::reference dst, type const& src);
  static bool unpack(type& dst, variant::const_reference src);
};

} /* namespace utils */
} /* namespace sysx */

#endif // SYSX_UNITS_COMMON_H_INCLUDED_

// include compatiblity with sc_trace, iff SystemC is available
// (deliberately outside of include guards!)
#if defined(SYSX_HAVE_SYSTEMC_)
#include "tvs/units/sc_trace.h"
#endif // SYSX_SYSTEMC_INCLUDED_
/* Taf!
 * :tag: (units,h)
 */
