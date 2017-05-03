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
#include "tvs/units/common.h"

/**
 * \file  common_impl.tpp
 * \brief template implementations for known units
 *
 * Although being template functions / classes, the implementations in
 * this file are not part of the installed headers, since only a
 * predefined set of units is supported.
 *
 * \see sysx::units, sysx::si, common_impl.cpp
 *
 */

#include <limits>

namespace sc_core {
class sc_time;
}

namespace sysx {
namespace units {

namespace impl {

template<typename Unit, typename ValueType = double>
struct parse_quantity
{
  typedef Unit unit_type;
  typedef ValueType value_type;
  typedef quantity<unit_type, value_type> quantity_type;

  static const quantity_type one;
  static const std::string unit_name;

  static std::string symbol();
  static quantity_type from_string(std::string const&);
  static quantity_type from_symbol(std::string const&);
};

template<typename T>
struct value_helper
{
  typedef T type;
  typedef T value_type;
  typedef std::numeric_limits<value_type> limits_type;

  static type max() { return limits_type::max(); }

  static type infinity() { return limits_type::infinity(); }

  static bool is_infinite(type const& v)
  {
    if (limits_type::has_infinity)
      return v == limits_type::infinity();
    return v == max();
  }
};

template<typename Unit, typename ValueType>
struct value_helper<quantity<Unit, ValueType>>
{
  typedef quantity<Unit, ValueType> type;
  typedef ValueType value_type;

  static type max()
  {
    return type::from_value(value_helper<value_type>::max());
  }

  static type infinity()
  {
    return type::from_value(value_helper<value_type>::infinity());
  }

  static bool is_infinite(type const& v)
  {
    return value_helper<value_type>::is_infinite(v.value());
  }
};

// forward declaration here - see sc_time.h
template<>
struct value_helper<sc_core::sc_time>;

} // namespace impl

template<typename T>
T
max_value()
{
  return impl::value_helper<T>::max();
}

template<typename T>
T
infinity()
{
  return impl::value_helper<T>::infinity();
}

template<typename T>
bool
is_infinite(T const& v)
{
  return impl::value_helper<T>::is_infinite(v);
}

template<typename Unit /*, typename ValueType*/>
quantity<Unit /*,ValueType*/>
from_string(std::string const& s)
{
  return impl::parse_quantity<Unit /*,ValueType*/>::from_string(s);
}

template<typename Unit /*, typename ValueType*/>
quantity<Unit /*,ValueType*/>
from_symbol(std::string const& u)
{
  return impl::parse_quantity<Unit /*,ValueType*/>::from_symbol(u);
}

/* ----------------------- implementation ------------------------ */

namespace impl {

template<typename Unit, typename ValueType>
std::string
parse_quantity<Unit, ValueType>::symbol()
{
  static const std::string sym = symbol_string(unit_type());
  return sym;
}

template<typename Unit, typename ValueType>
quantity<Unit, ValueType>
parse_quantity<Unit, ValueType>::from_string(std::string const& s)
{
  std::stringstream sstr(s);
  ValueType value;
  std::string unit;

  sstr >> value;

  // TODO: add proper error handling
  SYSX_ASSERT(!sstr.fail());

  sstr >> unit;
  SYSX_ASSERT(sstr.eof() && unit.size());

  return value * from_symbol(unit);
}

template<typename Unit, typename ValueType>
quantity<Unit, ValueType>
parse_quantity<Unit, ValueType>::from_symbol(std::string const& u)
{
  std::string::size_type pos = u.rfind(symbol());

  if (pos == std::string::npos || u.substr(pos) != symbol()) {
    SYSX_REPORT_ERROR(::sysx::report::invalid_unit_symbol) % u % unit_name %
      symbol();
  }

  return from_prefix(u.substr(0, pos)) * one;
}

} // namespace impl

} // namespace units
} // namespace sysx

namespace sysx {
namespace utils {

template<typename Unit, typename ValueType>
bool
variant_traits<units::quantity<Unit, ValueType>>::pack(variant::reference dst,
                                                       type const& src)
{
  typedef units::impl::parse_quantity<Unit, ValueType> helper;
  dst.set_list().push_back(src.value()).push_back(helper::symbol());
  return true;
}

template<typename Unit, typename ValueType>
bool
variant_traits<units::quantity<Unit, ValueType>>::unpack(
  type& dst,
  variant::const_reference src)
{
  typedef units::impl::parse_quantity<Unit, ValueType> helper;
  variant value, unit;

  // encoded as tuple
  if (src.is_list() && src.get_list().size() == 2) {
    variant::const_list_reference l = src.get_list();
    value = l[0];
    unit = l[1];
  }
  // encoded as map: "value"=, "unit"=
  else if (src.is_map() && src.get_map().size() == 2) {
    variant::const_map_reference m = src.get_map();
    if (m.has_entry("value") && m.has_entry("unit")) {
      value = m["value"];
      unit = m["unit"];
    }
  }
  // encoded as string
  else if (src.is_string()) {
    const char* str = src.get_string().c_str();
    char* endptr = NULL;
    double v = std::strtod(str, &endptr); // extract double value
    if (endptr == str || v < 0.0)
      return false;

    while (*endptr && std::isspace(*endptr))
      ++endptr; // skip whitespace
    if (!endptr)
      return false;

    value.set_double(v);
    unit.set_string(endptr);
  }

  ValueType v;
  if (!value.try_get(v) || !unit.is_string())
    return false;

  dst = v * helper::from_symbol(unit.get_string());
  return true;
}

} /* namespace utils */
} /* namespace sysx */

/* Taf!
 * :tag: (units,s)
 */
