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
 * \file   timed_variant.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  variant timed value type
 * \see    timed_value.h, variant.h
 */
#ifndef TVS_TIMED_VARIANT_H_INCLUDED_
#define TVS_TIMED_VARIANT_H_INCLUDED_

#include <tvs/tracing/timed_value.h>
#include <tvs/utils/variant.h>
#include <tvs/utils/variant_traits.h>

namespace tracing {

/**
 * \brief variant value with an associated duration
 *
 * \see timed_value, sysx::utils::variant
 */
class timed_variant : public timed_value<sysx::utils::variant>
{
  typedef timed_value<sysx::utils::variant> base_type;

public:

  /** \name constructors */
  ///\{

  /// default constructor - default value, infinite duration
  timed_variant()
    : base_type()
  {}

  /// value constructor - explicit value, infinite duration
  explicit timed_variant(value_type const& v)
    : base_type(v)
  {}

  /// value constructor - explicit arbitrary value, infinite duration
  template<typename T>
  explicit timed_variant(const T& v)
    : base_type(value_type(v))
  {}

  /// detailed constructor - explicit value, explicit duration
  timed_variant(value_type const& v, duration_type const& d)
    : base_type(v, d)
  {}

  /// detailed constructor - arbitrary value, explicit duration
  template<typename T>
  timed_variant(T const& v, duration_type const& d)
    : base_type(value_type(v), d)
  {}

  ///\}

}; // class timed_variant

} // namespace tracing

namespace sysx {
namespace utils {

template<>
struct variant_traits<tracing::timed_variant>
{
  typedef tracing::timed_variant type;

  static bool pack(variant::reference dst, type const& src)
  {
    variant_list ret;
    variant::list_reference dst_list = dst.set_list();

    ret.push_back(src.value());
    ret.push_back(src.duration());
    ret.swap(dst_list);

    return true;
  }
  static bool unpack(type& dst, variant::const_reference src)
  {
    if (!src.is_list())
      return false;

    variant::const_list_reference src_list = src.get_list();
    if (src_list.size() != 2)
      return false;

    // variant type, no try_*
    dst.value(src_list[0]);

    tracing::duration_type dur;

    if (!src_list[1].try_get(dur))
      return false;

    dst.duration(dur);

    return true;
  }
};

} // namespace utils
} // namespace sysx

#endif /* TVS_TIMED_VARIANT_H_INCLUDED_ */
/* Taf!
 */
