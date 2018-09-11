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

#include "tvs/utils/variant_traits.h"

#include <cctype>  // std::isspace
#include <cstdlib> // std::strtod

namespace sysx {
namespace utils {

/**
 * @file   variant_traits.cpp
 * @brief  conversions from and to a @ref variant (common types)
 * @author Philipp A. Hartmann, OFFIS
 */

#define DEFINE_PACK_(Type)                                                     \
  template<>                                                                   \
  bool variant_traits<Type>::pack(variant::reference dst, type const& src)

#define DEFINE_UNPACK_(Type)                                                   \
  template<>                                                                   \
  bool variant_traits<Type>::unpack(type& dst, variant::const_reference src)

// ----------------------------------------------------------------------------
// C++ builtin types

#define DEFINE_BUILTIN_(Type, Alias)                                           \
  DEFINE_PACK_(Type)                                                           \
  {                                                                            \
    dst.set_##Alias(src);                                                      \
    return true;                                                               \
  }                                                                            \
  DEFINE_UNPACK_(Type)                                                         \
  {                                                                            \
    if (!src.is_##Alias())                                                     \
      return false;                                                            \
    dst = src.get_##Alias();                                                   \
    return true;                                                               \
  }

DEFINE_BUILTIN_(bool, bool)
DEFINE_BUILTIN_(int, int)
DEFINE_BUILTIN_(int64, int64)
DEFINE_BUILTIN_(unsigned, uint)
DEFINE_BUILTIN_(uint64, uint64)
DEFINE_BUILTIN_(double, number)

DEFINE_BUILTIN_(std::string, string)

// ----------------------------------------------------------------------------
// SystemC builtin types

#if !defined(SYSX_NO_SYSTEMC)

DEFINE_PACK_(sc_core::sc_time)
{
#if SYSX_HAVE_SYSTEMC_ > SYSX_MAKE_VERSION(2, 3, 1)
  sc_core::sc_time_tuple t(src);
  if (t.has_value())
    dst.set_list().push_back(t.value()).push_back(std::string(t.unit_symbol()));
  else
    dst.set_list()
      .push_back(t.to_double())
      .push_back(std::string(t.unit_symbol()));
#else
  ///@todo normalize output to best matching unit
  dst.set_list().push_back(src.to_seconds()).push_back("s");
#endif // SystemC > 2.3.1
  return true;
}

static inline sc_core::sc_time
sc_time_from_unit(double value, unsigned unit)
{
  ///@todo add error handling
  SYSX_ASSERT(value >= 0.0);
  SYSX_ASSERT(unit <= sc_core::SC_SEC);
  auto tu = static_cast<sc_core::sc_time_unit>(unit);
  return sc_core::sc_time(value, tu);
}

static inline sc_core::sc_time
sc_time_from_symbol(double value, variant_string_cref sym)
{
#if SYSX_HAVE_SYSTEMC_ > SYSX_MAKE_VERSION(2, 3, 1)
  return sc_core::sc_time(value, sym.c_str());
#else  // SystemC <= 2.3.1
  static struct symbol
  {
    const char* sym[2];
    sc_core::sc_time_unit unit;
  } symbol_table[] = { { { "fs", "SC_FS" }, sc_core::SC_FS },
                       { { "ps", "SC_PS" }, sc_core::SC_PS },
                       { { "ns", "SC_NS" }, sc_core::SC_NS },
                       { { "us", "SC_US" }, sc_core::SC_US },
                       { { "ms", "SC_MS" }, sc_core::SC_MS },
                       { { "s", "SC_SEC" }, sc_core::SC_SEC },
                       { { "sec" }, sc_core::SC_SEC },
                       { { NULL, NULL }, sc_core::sc_time_unit() } };
  symbol* cursor = symbol_table;
  while (*cursor->sym && !(sym == cursor->sym[0] || sym == cursor->sym[1]))
    cursor++;

  SYSX_ASSERT(unsigned(cursor - symbol_table) <
            (sizeof(symbol_table) / sizeof(symbol)));
  return sc_time_from_unit(value, unsigned(cursor - symbol_table));
#endif // SystemC > 2.3.1
}

DEFINE_UNPACK_(sc_core::sc_time)
{
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
    char* endptr = nullptr;
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

  if (!value.is_number())
    return false;

  if (unit.is_string()) {
    dst = sc_time_from_symbol(value.get_double(), unit.get_string());
    return true;
  } else if (unit.is_uint() && unit.get_uint() <= sc_core::SC_SEC) {
    dst = sc_time_from_unit(value.get_double(), unit.get_uint());
    return true;
  }
  return false;
}

DEFINE_BUILTIN_(sc_dt::sc_bit, bool)

DEFINE_PACK_(sc_dt::sc_logic)
{
  switch (src.value()) {
    case sc_dt::Log_0:
    case sc_dt::Log_1:
      dst.set_bool(src.value());
      break;
    default:
      dst.set_string(std::string(1, src.to_char()));
      break;
  }
  return true;
}

DEFINE_UNPACK_(sc_dt::sc_logic)
{
  if (src.is_bool()) {
    dst = src.get_bool();
    return true;
  } else if (src.is_int()) {
    dst = sc_dt::sc_logic(src.get_int());
    return true;
  } else if (src.is_string() && src.get_string().size() == 1) {
    switch (src.get_string()[0]) {
      case '0':
        dst = sc_dt::SC_LOGIC_0;
        return true;
      case '1':
        dst = sc_dt::SC_LOGIC_1;
        return true;
      case 'z': /* fallthrough */
      case 'Z':
        dst = sc_dt::SC_LOGIC_Z;
        return true;
      case 'x': /* fallthrough */
      case 'X':
        dst = sc_dt::SC_LOGIC_X;
        return true;
      default: /* nothing */;
    }
  }
  return false;
}

DEFINE_PACK_(sc_dt::sc_int_base)
{
  dst.set_int64(src);
  return true;
}

DEFINE_UNPACK_(sc_dt::sc_int_base)
{
  if (src.is_int64())
    dst = src.get_int64();
  else if (src.is_string())
    dst = src.get_string().c_str();
  else
    return false;

  return true;
}

DEFINE_PACK_(sc_dt::sc_uint_base)
{
  dst.set_uint64(src);
  return true;
}

DEFINE_UNPACK_(sc_dt::sc_uint_base)
{
  if (src.is_uint64())
    dst = src.get_uint64();
  else if (src.is_string())
    dst = src.get_string().c_str();
  else
    return false;

  return true;
}

DEFINE_PACK_(sc_dt::sc_signed)
{
  if (src.length() <= 64)
    dst.set_int64(src.to_int64());
  else
    dst.set_string(src.to_string());

  return true;
}

DEFINE_UNPACK_(sc_dt::sc_signed)
{
  ///@todo add bounds checks
  if (src.is_int64())
    dst = src.get_int64();
  else if (src.is_string())
    dst = src.get_string().c_str();
  else
    return false;

  return true;
}

DEFINE_PACK_(sc_dt::sc_unsigned)
{
  if (src.length() <= 64)
    dst.set_uint64(src.to_uint64());
  else
    dst.set_string(src.to_string());

  return true;
}

DEFINE_UNPACK_(sc_dt::sc_unsigned)
{
  ///@todo add bounds checks
  if (src.is_uint64())
    dst = src.get_uint64();
  else if (src.is_string())
    dst = src.get_string().c_str();
  else
    return false;

  return true;
}

DEFINE_PACK_(sc_dt::sc_bv_base)
{
  dst.set_string(src.to_string());
  return true;
}

DEFINE_UNPACK_(sc_dt::sc_bv_base)
{
  ///@todo add bounds checks
  if (!src.is_string())
    return false;

  dst = src.get_string().c_str();
  return true;
}

DEFINE_PACK_(sc_dt::sc_lv_base)
{
  dst.set_string(src.to_string());
  return true;
}

DEFINE_UNPACK_(sc_dt::sc_lv_base)
{
  ///@todo add bounds checks
  if (!src.is_string())
    return false;

  dst = src.get_string().c_str();
  return true;
}

#endif // SYSX_NO_SYSTEMC

} /* namespace utils */
} /* namespace sysx */
/* Taf!
 * :tag: (variant,s)
 */
