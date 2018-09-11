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

#ifndef SYSX_UTILS_VARIANT_TRAITS_H_INCLUDED_
#define SYSX_UTILS_VARIANT_TRAITS_H_INCLUDED_

#include "tvs/utils/systemc.h"
#include "tvs/utils/variant.h"

#include <cstring> // std::strncpy

// forward declarations for some C++ std classes/templates
namespace std {
template<typename T1, typename T2>
struct pair;
template<typename T1, typename Alloc>
class vector;
template<typename T, typename Key, typename Alloc>
class set;
} // namespace std

namespace tracing {
class timed_duration;
class timed_variant;

template<typename T1>
struct timed_value;
}

/**
 * @file   variant_traits.h
 * @brief  conversions from and to a @ref variant
 * @author Philipp A. Hartmann, OFFIS
 */

namespace sysx {
namespace utils {

/**
 * @class variant_traits
 * @brief traits class for variant conversions
 * @tparam T C++ datatype to convert to/from @ref variant
 *
 * Whenever a C++ type @c T is used in conjunction with a variant,
 * the requireed value conversion is performed by this traits class,
 * providing the two conversion functions @ref pack and @ref unpack.
 * Both functions return @c true upon success and @c false otherwise.
 * In case of a failing conversion, it is recommended to leave the given
 * destination object @c dst untouched.
 *
 * @note By default, the primary template is not implemented to
 *       enable instantiations with incomplete types.
 *
 * You only need to implement the two functions @ref pack / @ref unpack
 * to enable conversion support for your custom datatype:
 * @code
 * struct my_int { int value; };
 *
 * template<> bool
 * variant_traits<my_int>::pack( variant::reference dst, type const & src )
 * {
 *    dst.set_int( src.value );
 *    return true;
 * }
 * template<> bool
 * variant_traits<my_int>::unpack( type & dst, variant::const_reference src )
 * {
 *    if( ! src.is_int() ) return false;
 *    dst.value  = src.get_int();
 *    return true;
 * }
 * @endcode
 *
 * To @em disable conversion support for a given type, you can refer
 * to the helper template @ref variant_traits_disabled.
 */
template<typename T>
struct variant_traits
{
  typedef T type; ///< common type alias
  /// convert from \ref type value to a \ref variant
  static bool pack(variant::reference dst, type const& src);
  /// convert from \ref variant to a \ref type value
  static bool unpack(type& dst, variant::const_reference src);
};

// ---------------------------------------------------------------------------
/**
 * @brief helper to disable variant conversion for a given type
 * @tparam T type without variant conversions
 *
 * In order to disable the conversion from/to a variant for a given type
 * @c T during @em run-time, you can simply inherit from this helper in
 * the specialization of variant_traits:
 * @code
 * struct my_type { ... };
 * template<>
 * struct variant_traits<my_type>
 *   : variant_traits_disabled<my_type> {};
 * @endcode
 *
 * \note In order to disable support for a given type at @em compile-time,
 *       the specialization of variant_traits can be left empty.
 */
template<typename T>
struct variant_traits_disabled
{
  typedef T type;
  static bool pack(variant::reference, T const&) { return false; }
  static bool unpack(type&, variant::const_reference) { return false; }
};

///@cond SYSX_HIDDEN_FROM_DOXYGEN

// ---------------------------------------------------------------------------
// disabled implementation as a safety guard

// clang-format off
template<typename T> struct variant_traits<T*>      { /* disabled */ };
template<> struct variant_traits<variant>           { /* disabled */ };
template<> struct variant_traits<variant_cref>      { /* disabled */ };
template<> struct variant_traits<variant_ref>       { /* disabled */ };
template<> struct variant_traits<variant_list>      { /* disabled */ };
template<> struct variant_traits<variant_list_cref> { /* disabled */ };
template<> struct variant_traits<variant_list_ref>  { /* disabled */ };
// clang-format on

// ---------------------------------------------------------------------------
/// helper to convert compatible types (implementation artefact)
template<typename T, typename U>
struct variant_traits_convert
{
  typedef T type;
  typedef variant_traits<U> traits_type;

  static bool pack(variant::reference dst, type const& src)
  {
    return traits_type::pack(dst, U(src));
  }

  static bool unpack(type& dst, variant::const_reference src)
  {
    U u_dst;
    bool ret = traits_type::unpack(u_dst, src);
    if (ret)
      dst = u_dst;
    return ret;
  }
};

// --------------------------------------------------------------------------
// C++ builtin types

// default instantiations (in variant_traits.cpp)
template struct variant_traits<bool>;
template struct variant_traits<int>;
template struct variant_traits<int64>;
template struct variant_traits<unsigned>;
template struct variant_traits<uint64>;
template struct variant_traits<double>;
template struct variant_traits<std::string>;

// related numerical types
// (without range checks for now)

#define VARIANT_TRAITS_DERIVED_(UnderlyingType, SpecializedType)               \
  template<>                                                                   \
  struct variant_traits<SpecializedType>                                       \
    : variant_traits_convert<SpecializedType, UnderlyingType>                  \
  {                                                                            \
  }

VARIANT_TRAITS_DERIVED_(int, char);
VARIANT_TRAITS_DERIVED_(int, signed char);
VARIANT_TRAITS_DERIVED_(int, short);
VARIANT_TRAITS_DERIVED_(unsigned, unsigned char);
VARIANT_TRAITS_DERIVED_(unsigned, unsigned short);
VARIANT_TRAITS_DERIVED_(int64, long);
VARIANT_TRAITS_DERIVED_(uint64, unsigned long);
VARIANT_TRAITS_DERIVED_(double, float);

///@endcond SYSX_HIDDEN_FROM_DOXYGEN

// ----------------------------------------------------------------------------
// C++ string literals

template<int N>
struct variant_traits<char[N]>
{
  typedef char type[N]; ///< common type alias
  static bool pack(variant::reference dst, type const& src)
  {
    dst.set_string(src);
    return true;
  }
  static bool unpack(type& dst, variant::const_reference src)
  {
    if (src.is_null()) {
      dst[0] = '\0'; // convert "null" to empty string
      return true;
    }
    if (!src.is_string())
      return false;

    variant::const_string_reference str = src.get_string();
    std::strncpy(dst, str, N - 1);
    dst[N - 1] = '\0';
    return (str.size() <= N - 1);
  }
};

// ----------------------------------------------------------------------------
// C++ arrays

template<typename T, int N>
struct variant_traits<T[N]>
{
  typedef T type[N]; ///< common type alias
  static bool pack(variant::reference dst, type const& src)
  {
    variant_list ret;
    ret.reserve(N);

    for (size_t i = 0; i < N; ++i)
      ret.push_back(src[i]);
    ret.swap(dst.set_list());
    return true;
  }
  static bool unpack(type& dst, variant::const_reference src)
  {
    if (!src.is_list())
      return false;

    variant::const_list_reference lst = src.get_list();
    size_t i = 0;
    for (; i < N && i < lst.size() && lst[i].try_get<T>(dst[i]); ++i) {
    }

    return (i == lst.size());
  }
};

// ----------------------------------------------------------------------------
// std::pair<T1, T2>

template<typename T1, typename T2>
struct variant_traits<std::pair<T1, T2>>
{
  typedef std::pair<T1, T2> type; ///< common type alias
  static bool pack(variant::reference dst, type const& src)
  {
    variant_list ret;
    variant::list_reference dst_list = dst.set_list();

    ret.push_back(src.first).push_back(src.second).swap(dst_list);
    return true;
  }
  static bool unpack(type& dst, variant::const_reference src)
  {
    if (!src.is_list())
      return false;

    variant::const_list_reference src_list = src.get_list();
    if (src_list.size() != 2)
      return false;

    // extract T1, T2 values via templated try_get() functions
    if (!src_list[0].try_get(dst.first) || !src_list[1].try_get(dst.second))
      return false;

    return true;
  }
};

// ----------------------------------------------------------------------------
// std::vector<T, Alloc>

template<typename T, typename Alloc>
struct variant_traits<std::vector<T, Alloc>>
{
  typedef std::vector<T, Alloc> type; ///< common type alias
  static bool pack(variant::reference dst, type const& src)
  {
    variant_list ret;
    ret.reserve(src.size());

    for (size_t i = 0; i < src.size(); ++i)
      ret.push_back(src[i]);
    ret.swap(dst.set_list());
    return true;
  }
  static bool unpack(type& dst, variant::const_reference src)
  {
    if (!src.is_list())
      return false;

    variant::const_list_reference lst = src.get_list();
    type ret;
    T cur;
    size_t i = 0;
    ret.reserve(lst.size());
    for (; i < lst.size() && lst[i].try_get(cur); ++i)
      ret.push_back(cur);

    return (i == lst.size()) ? (dst.swap(ret), true) : false;
  }
};

template<typename T, typename Compare, typename Alloc>
struct variant_traits<std::set<T, Compare, Alloc>>
{
  typedef std::set<T, Compare, Alloc> type; ///< common type alias
  static bool pack(variant::reference dst, type const& src)
  {
    variant_list ret;
    ret.reserve(src.size());

    for (auto&& i : src)
      ret.push_back(i);
    ret.swap(dst.set_list());
    return true;
  }
  static bool unpack(type& dst, variant::const_reference src)
  {
    if (!src.is_list())
      return false;

    variant::const_list_reference lst = src.get_list();
    type ret;
    T cur;
    size_t i = 0;
    for (; i < lst.size() && lst[i].try_get(cur); ++i)
      ret.insert(cur);

    return (i == lst.size()) ? (dst.swap(ret), true) : false;
  }
};

// ----------------------------------------------------------------------------
// SystemC builtin types

#if !defined(SYSX_NO_SYSTEMC)

// default instantiations (in variant_traits.cpp)
template struct variant_traits<sc_core::sc_time>;
template struct variant_traits<sc_dt::sc_logic>;
template struct variant_traits<sc_dt::sc_int_base>;
template struct variant_traits<sc_dt::sc_uint_base>;
template struct variant_traits<sc_dt::sc_signed>;
template struct variant_traits<sc_dt::sc_unsigned>;
template struct variant_traits<sc_dt::sc_bv_base>;
template struct variant_traits<sc_dt::sc_lv_base>;

template<int N>
struct variant_traits<sc_dt::sc_int<N>> : variant_traits<sc_dt::sc_int_base>
{
  typedef sc_dt::sc_int<N> type;
  /// @todo add bounds checks
};

template<int N>
struct variant_traits<sc_dt::sc_uint<N>> : variant_traits<sc_dt::sc_uint_base>
{
  typedef sc_dt::sc_uint<N> type;
  /// @todo add bounds checks
};

template<int N>
struct variant_traits<sc_dt::sc_bigint<N>> : variant_traits<sc_dt::sc_signed>
{
  typedef sc_dt::sc_bigint<N> type;
  /// @todo add bounds checks
};

template<int N>
struct variant_traits<sc_dt::sc_biguint<N>> : variant_traits<sc_dt::sc_unsigned>
{
  typedef sc_dt::sc_biguint<N> type;
  /// @todo add bounds checks
};

template<int N>
struct variant_traits<sc_dt::sc_bv<N>> : variant_traits<sc_dt::sc_bv_base>
{
  typedef sc_dt::sc_bv<N> type;
  /// @todo add bounds checks
};

template<int N>
struct variant_traits<sc_dt::sc_lv<N>> : variant_traits<sc_dt::sc_lv_base>
{
  typedef sc_dt::sc_lv<N> type;
  /// @todo add bounds checks
};

#endif // SYSX_NO_SYSTEMC

// ----------------------------------------------------------------------------
// Timed-Value Stream types

template<typename T>
struct variant_traits<tracing::timed_value<T>>
{
  typedef tracing::timed_value<T> type;
  typedef std::pair<T, tracing::timed_duration> pair_type;

  static bool pack(variant::reference dst, type const& src)
  {
    pair_type p = std::make_pair(src.value(), src.duration());
    return variant_traits<pair_type>::pack(dst, p);
  }
  static bool unpack(type& dst, variant::const_reference src)
  {
    pair_type p;
    if (variant_traits<pair_type>::unpack(p, src)) {
      dst.value(p.first);
      dst.duration(p.second);
      return true;
    }
    return false;
  }
};

} /* namespace utils */
} /* namespace sysx */

#endif // SYSX_UTILS_VARIANT_TRAITS_H_INCLUDED_

#if !defined(SYSX_NO_SYSTEMC)

///@todo add support for SystemC fixpoint types
#if defined(SC_INCLUDE_FX) && !defined(SYSX_UTILS_VARIANT_TRAITS_H_INCLUDED_FX_)
#define SYSX_UTILS_VARIANT_TRAITS_H_INCLUDED_FX_
namespace sysx {
namespace utils {

template struct variant_traits<sc_dt::sc_fxval>;
template struct variant_traits<sc_dt::sc_fxval_fast>;
template struct variant_traits<sc_dt::sc_fix>;
template struct variant_traits<sc_dt::sc_ufix>;
template struct variant_traits<sc_dt::sc_fixed_fast>;

template<int W, int I, sc_dt::sc_q_mode, sc_dt::sc_o_mode O, int N>
struct variant_traits<sc_dt::sc_fixed<W, I, Q, O, N>> : variant_traits<sc_fix>
{
  typedef sc_dt::sc_fixed<W, I, Q, O, N> type;
  /// @todo add bounds checks
};

template<int W, int I, sc_dt::sc_q_mode, sc_dt::sc_o_mode O, int N>
struct variant_traits<sc_dt::sc_ufixed<W, I, Q, O, N>> : variant_traits<sc_ufix>
{
  typedef sc_dt::sc_ufixed<W, I, Q, O, N> type;
  /// @todo add bounds checks
};

} /* namespace utils */
} /* namespace sysx */
#endif // SC_INCLUDE_FX && ! SYSX_UTILS_VARIANT_TRAITS_H_INCLUDED_FX_

#endif // SYSX_NO_SYSTEMC

#ifdef VARIANT_TRAITS_DERIVED_
#undef VARIANT_TRAITS_DERIVED_
#endif
/* Taf!
 * :tag: (variant,h)
 */
