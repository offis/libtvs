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

/**
 * \brief  VCD traits
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>,
 *         Sören Schreiner <soeren.schreiner@offis.de>
 */

#include <boost/format.hpp>

#include <algorithm>
#include <bitset>
#include <climits>
#include <iomanip>
#include <ostream>

template<typename T>
struct vcd_traits
{
  using value_type = T;
  using this_type = vcd_traits<value_type>;

  static constexpr const char* header_identifier_value = "real";
  static constexpr const char* trace_identifier_value = "r";
  static constexpr uint16_t bitwidth_value = sizeof(value_type) * CHAR_BIT;

  static void print(std::ostream& out, value_type const& val) { out << val; }
};

template struct vcd_traits<uint16_t>;
template struct vcd_traits<uint32_t>;
template struct vcd_traits<int>;

template<>
struct vcd_traits<bool>
{
  using value_type = bool;
  using this_type = vcd_traits<value_type>;

  static constexpr const char* header_identifier_value = "wire";
  static constexpr const char* trace_identifier_value = "b";
  static constexpr uint16_t bitwidth_value = 1;

  static void print(std::ostream& out, value_type const& val)
  {
    out << (val ? "1" : "0");
  }
};

template<>
struct vcd_traits<double>
{
  using value_type = double;
  using this_type = vcd_traits<value_type>;

  static constexpr const char* header_identifier_value = "real";
  static constexpr const char* trace_identifier_value = "r";
  static constexpr uint16_t bitwidth_value = sizeof(value_type) * CHAR_BIT;

  static void print(std::ostream& out, value_type const& val)
  {
    out << boost::format("%.16g") % val;
  }
};

template<>
struct vcd_traits<std::string>
{
  using value_type = std::string;
  using this_type = vcd_traits<value_type>;

  static constexpr const char* header_identifier_value = "wire";
  static constexpr const char* trace_identifier_value = "b";
  static constexpr uint16_t bitwidth_value = 800;

  static void print(std::ostream& out, value_type const& val)
  {
    for (std::size_t i = 0; i < bitwidth_value/8; i++) {
      char data;
      if (i < val.size()) {
        data = val[i];
      } else {
        data = 0;
      }
      out << std::bitset<8>(data);
    }
  }
};
