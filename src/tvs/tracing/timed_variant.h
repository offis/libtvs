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

#include "tvs/tracing/timed_value.h"
#include "tvs/utils/variant.h"
#include "tvs/utils/variant_traits.h"

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
  void operator=(base_type const& rhs)
  {
    this->value(rhs.value());
    this->duration(rhs.duration());
  }

  /** \name constructors */
  ///\{

  /// default constructor - default value, infinite duration
  timed_variant()
    : base_type()
  {
  }

  /// value constructor - explicit value, infinite duration
  explicit timed_variant(value_type const& v)
    : base_type(v)
  {
  }

  /// value constructor - explicit arbitrary value, infinite duration
  template <typename T>
  explicit timed_variant(const T& v)
    : base_type(value_type(v))
  {
  }

  /// detailed constructor - explicit value, explicit duration
  timed_variant(value_type const& v, duration_type const& d)
    : base_type(v, d)
  {
  }

  /// detailed constructor - arbitrary value, explicit duration
  template <typename T>
  timed_variant(T const& v, duration_type const& d)
    : base_type(value_type(v), d)
  {
  }

  ///\}

}; // class timed_variant

} // namespace tracing

namespace sysx {
namespace utils {

#define VARIANT_TRAITS_DERIVED_(UnderlyingType, SpecializedType)               \
  template <>                                                                  \
  struct variant_traits<SpecializedType>                                       \
    : variant_traits_convert<SpecializedType, UnderlyingType>                  \
  {                                                                            \
  }

VARIANT_TRAITS_DERIVED_(tracing::timed_value<sysx::utils::variant>,
                        tracing::timed_variant);

#undef VARIANT_TRAITS_DERIVED_

#if 0
template <>
struct variant_traits<tracing::timed_variant>
{
  typedef tracing::timed_variant type;
  typedef tracing::timed_value<sysx::utils::variant> tv_type;

  typedef variant_traits<tv_type> variant_type;

  static bool pack(variant::reference dst, type const& src)
  {
    tv_type tuple(src.value(), src.duration());
    return variant_type::pack(dst, tuple);
  }

  static bool unpack(type& dst, variant::const_reference src)
  {
    tv_type tuple;
    if (variant_type::unpack(tuple, src)) {
      dst.value(tuple.value());
      dst.duration(tuple.duration());
      return true;
    }
  }
};

#endif

} // namespace utils
} // namespace sysx

#endif /* TVS_TIMED_VARIANT_H_INCLUDED_ */
/* Taf!
 */
