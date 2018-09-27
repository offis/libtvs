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

#ifndef TVS_VCD_TRAITS_H_INCLUDED_
#define TVS_VCD_TRAITS_H_INCLUDED_

#include <climits>
#include <ostream>

namespace tracing {

template<typename T>
struct vcd_traits
{
  using value_type = T;
  using this_type = vcd_traits<value_type>;
  static void print(std::ostream& out, value_type const& val);
  static const char* header_id();
  static const char* trace_id();
  static uint16_t bitwidth();
};

// these are provided by the library

template struct vcd_traits<int>;
template struct vcd_traits<bool>;
template struct vcd_traits<double>;
template struct vcd_traits<std::string>;

} // namespace tracing

#endif /* TVS_VCD_TRAITS_H_INCLUDED_ */
