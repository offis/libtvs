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

#ifndef SYSX_UTILS_VARIANT_H_INCLUDED_
#define SYSX_UTILS_VARIANT_H_INCLUDED_

#include "tvs/utils/report.h"
#include "tvs/utils/report_msgs.h"
#include "tvs/utils/systemc.h" // sc_dt::(u)int64, potentially strip out

#include <cstring> // std::strlen

/**
 * @file   variant.h
 * @brief a simple variant datatype
 * @author Philipp A. Hartmann, OFFIS
 */

namespace sysx {
namespace utils {

// define our own typedefs to avoid SystemC dependency?
using sc_dt::int64;
using sc_dt::uint64;

// forward declarations
class variant;
class variant_cref;
class variant_ref;
class variant_string_cref;
class variant_string_ref;
class variant_list;
class variant_list_cref;
class variant_list_ref;
class variant_map;
class variant_map_cref;
class variant_map_ref;

template<typename T>
struct variant_traits;

///@cond SYSX_HIDDEN_FROM_DOXYGEN
#define SYSX_VARIANT_TRAITS_(Type) typename variant_traits<Type>::type
#define SYSX_VARIANT_TRAITS_ENABLED_(Type) SYSX_VARIANT_TRAITS_(Type)*
#define SYSX_VARIANT_ENABLE_IF_TRAITS_(Type)                                   \
  SYSX_VARIANT_TRAITS_ENABLED_(Type) = 0
///@endcond SYSX_HIDDEN_FROM_DOXYGEN

/// @ref variant comparisons
bool
operator==(variant_cref const&, variant_cref const&);

// --------------------------------------------------------------------------
/// @ref variant value category
enum variant_category
{
  VARIANT_NULL = 0,
  VARIANT_BOOL,
  VARIANT_NUMBER,
  VARIANT_STRING,
  VARIANT_LIST,
  VARIANT_MAP
};

/// reference to a constant (nested) @ref variant
class variant_cref
{
  friend class variant_ref;
  friend class variant_list_cref;
  friend class variant_list_ref;
  friend class variant_map_cref;
  friend class variant_map_ref;
  friend bool operator==(variant_cref const&, variant_cref const&);

protected:
  typedef void impl; // use type-punned pointer for now

  explicit variant_cref(impl* i = NULL)
    : pimpl_(i)
  {
  }

public:
  /** @name type queries */
  ///@{
  variant_category category() const;
  bool is_null() const;

  bool is_bool() const;
  bool is_false() const { return is_bool() && !get_bool(); }
  bool is_true() const { return is_bool() && get_bool(); }

  bool is_number() const { return is_int64() || is_uint64() || is_double(); }

  bool is_int() const;
  bool is_uint() const;
  bool is_int64() const;
  bool is_uint64() const;
  bool is_double() const;

  bool is_string() const;

  bool is_map() const;

  bool is_list() const;
  //@}

  /** @name value queries */
  //@{

  /// get boolean value
  bool get_bool() const;

  /// get integer value
  int get_int() const;
  /// get unsigned integer value
  unsigned get_uint() const;
  /// get 64-bit integer value
  int64 get_int64() const;
  /// get 64-bit unsigned integer value
  uint64 get_uint64() const;
  /// get floating point value
  double get_double() const;
  /// get the numeric value (alias for get_double())
  double get_number() const { return get_double(); }
  //@}

  /**
   * @name complex value queries
   * These functions return (constant) references to the complex value types
   * that can be stored in a variant (strings, lists, maps).
   */
  //@{
  variant_string_cref get_string() const;
  variant_list_cref get_list() const;
  variant_map_cref get_map() const;
  //@}

  /** @name arbitrary type value queries */
  //@{
  /// try to get a value of a @ref variant_traits enabled type
  template<typename T>
  bool try_get(T& dst, SYSX_VARIANT_ENABLE_IF_TRAITS_(T)) const;
  /// get a value of a @ref variant_traits enabled type
  template<typename T>
  SYSX_VARIANT_TRAITS_(T)
  get() const;
  //@}

  /// convert value to JSON
  bool json_serialize(std::string&) const;

protected:
  impl* pimpl_;

private:
  /// constant reference, disabled assignment
  variant_cref operator=(variant_cref const&) /* = delete */;
};

template<typename T>
bool
variant_cref::try_get(T& dst, SYSX_VARIANT_TRAITS_ENABLED_(T)) const
{
  typedef variant_traits<T> traits;
  return traits::unpack(dst, *this);
}

template<typename T>
SYSX_VARIANT_TRAITS_(T)
variant_cref::get() const
{
  T result;
  if (!try_get(result)) {
    SYSX_REPORT_ERROR(report::variant_error)
      << "conversion from variant failed";
  }
  return result;
}

// --------------------------------------------------------------------------

/// reference to a mutable (nested) @ref variant
class variant_ref : public variant_cref
{
  friend class variant_string_ref;
  friend class variant_list_ref;
  friend class variant_map_ref;
  typedef variant_cref base_type;
  typedef variant_ref this_type;

protected:
  explicit variant_ref(impl* i = NULL)
    : variant_cref(i)
  {
  }

public:
  /// exchange contents with another variant
  void swap(this_type& that);
  this_type operator=(const base_type&);
  this_type operator=(const this_type&);

  /** @name set value functions
   *
   * The various @c set_* functions update the represented value
   * (and its @ref basic_type()) accordingly.
   *
   * These functions provide a "fluent interface" by returning a reference
   * to the surrounding variant object.  This enables chained function calls,
   * which is especially convenient for complex values like lists and maps.
   */
  ///@{

  /// set value to variant_traits enabled type
  template<typename T>
  variant_ref set(T const& dst, SYSX_VARIANT_ENABLE_IF_TRAITS_(T));
  /// try to set value to variant_traits enabled type
  template<typename T>
  bool try_set(T const& dst, SYSX_VARIANT_ENABLE_IF_TRAITS_(T));

  /// set value to @c null
  variant_ref set_null();
  /// set boolean value (@c true, @c false)
  variant_ref set_bool(bool v);
  /// set integer value
  variant_ref set_int(int v);
  /// set unsigned integer value
  variant_ref set_uint(unsigned v);
  /// set 64-bit integer value
  variant_ref set_int64(int64 v);
  /// set unsigned 64-bit integer value
  variant_ref set_uint64(uint64 v);
  /// set double value
  variant_ref set_double(double v);
  /// set double value (alias for set_double())
  variant_ref set_number(double v) { return set_double(v); }

  /// set string value from null-terminated string
  variant_string_ref set_string(const char* s);
  /// set string value from std::string
  variant_string_ref set_string(const std::string& s);
  /// set string value from variant::const_string_reference
  variant_string_ref set_string(variant_string_cref s);
  /// set string value from null-terminated string and length
  variant_string_ref set_string(const char* s, size_t len);

  /// set value to an (empty) list
  variant_list_ref set_list();
  /// set value to an (empty) map
  variant_map_ref set_map();
  ///@}

  /** @name complex value queries
   *
   * The functions return a reference to a (mutable) @ref variant object
   * of the corresponding complex value type (string, list, map).
   */
  ///@{
  using base_type::get_string;
  variant_string_ref get_string();

  using base_type::get_list;
  variant_list_ref get_list();

  using base_type::get_map;
  variant_map_ref get_map();
  ///@}

  /// try to set the value from a JSON-encoded string
  bool json_deserialize(std::string const&);
};

inline variant_ref
variant_ref::operator=(this_type const& that)
{
  return *this = base_type(that);
}

template<typename T>
bool
variant_ref::try_set(T const& src, SYSX_VARIANT_TRAITS_ENABLED_(T))
{
  typedef variant_traits<T> traits;
  return traits::pack(*this, src);
}

template<typename T>
variant_ref
variant_ref::set(T const& src, SYSX_VARIANT_TRAITS_ENABLED_(T))
{
  if (!try_set(src)) {
    SYSX_REPORT_ERROR(report::variant_error) << "conversion to variant failed";
  }
  return *this;
}

// --------------------------------------------------------------------------

// clang-format off
/** @name variant string comparisons */
///@{
bool operator==( const char *, variant_string_cref const & );
bool operator==( variant_string_cref const &, const char * );
bool operator==( std::string const &, variant_string_cref const & );
bool operator==( variant_string_cref const &, std::string const & );
///@}
// clang-format on

/// reference to constant variant string value
class variant_string_cref : public variant_cref
{
  friend class variant_cref;
  friend bool operator==(const char*, variant_string_cref const&);
  friend bool operator==(variant_string_cref const&, const char*);
  friend bool operator==(std::string const&, variant_string_cref const&);
  friend bool operator==(variant_string_cref const&, std::string const&);
  typedef variant_cref base_type;
  typedef variant_string_cref this_type;

protected:
  explicit variant_string_cref(impl* i = NULL)
    : base_type(i)
  {
  }

public:
  typedef size_t size_type;

  /// empty string?
  bool empty() const { return size() == 0; }
  size_type length() const { return size(); }
  size_type size() const;

  /// implicit conversion to std::string
  operator std::string() const { return std::string(c_str(), length()); }
  /// obtain underlying null-terminated string
  char const* c_str() const;

  /// character access by index
  char operator[](size_type index) const { return c_str()[index]; }

private:
  // exclude non-string value functions
  using base_type::get_bool;
  using base_type::get_int;
  using base_type::get_uint;
  using base_type::get_int64;
  using base_type::get_uint64;
  using base_type::get_double;
  using base_type::get_list;
  using base_type::get_map;

private:
  // constant reference, no assignment
  this_type& operator=(this_type const&) /* = delete */;
};

// --------------------------------------------------------------------------

/// reference to mutable variant string value
class variant_string_ref : public variant_string_cref
{
  friend class variant_ref;
  typedef variant_string_cref base_type;
  typedef variant_string_ref this_type;

protected:
  explicit variant_string_ref(impl* i = NULL)
    : base_type(i)
  {
  }

public:
  /// exchange contents with another string value
  void swap(this_type& that);

  /** @name assign string contents */
  //@{
  this_type operator=(this_type const&);
  this_type operator=(variant_string_cref s);
  this_type operator=(const char* s);
  this_type operator=(std::string const& s);
  //@}
};

inline variant_string_ref
variant_string_ref::operator=(this_type const& s)
{
  return variant_ref(pimpl_).set_string(s);
}

inline variant_string_ref
variant_string_ref::operator=(variant_string_cref s)
{
  return variant_ref(pimpl_).set_string(s);
}

inline variant_string_ref
variant_string_ref::operator=(const char* s)
{
  return variant_ref(pimpl_).set_string(s);
}

inline variant_string_ref
variant_string_ref::operator=(std::string const& s)
{
  return variant_ref(pimpl_).set_string(s);
}

inline variant_string_ref
variant_ref::get_string()
{
  return variant_string_ref(base_type::get_string().pimpl_);
}

inline variant_string_ref
variant_ref::set_string(char const* s)
{
  return set_string(s ? s : "", s ? std::strlen(s) : 0u);
}

inline variant_string_ref
variant_ref::set_string(variant_string_cref s)
{
  return set_string(s.c_str(), s.length());
}

inline variant_string_ref
variant_ref::set_string(std::string const& s)
{
  return set_string(s.c_str(), s.length());
}

// --------------------------------------------------------------------------

/// reference to constant variant list value
class variant_list_cref : public variant_cref
{
  friend class variant_cref;
  typedef variant_cref base_type;
  typedef variant_list_cref this_type;

protected:
  explicit variant_list_cref(impl* i = NULL)
    : base_type(i)
  {
  }

public:
  typedef size_t size_type;
  typedef variant_cref const_reference;
  typedef variant_ref reference;

  /** @name list queries */
  //@{
  bool empty() const { return size() == 0; }
  size_type size() const;
  //@}

  /** @name (constant) element access by index */
  //@{
  const_reference operator[](size_type index) const;
  const_reference at(size_type index) const { return (*this)[index]; }
  //@}

private:
  // exclude non-list value functions
  using base_type::get_bool;
  using base_type::get_int;
  using base_type::get_uint;
  using base_type::get_int64;
  using base_type::get_uint64;
  using base_type::get_double;
  using base_type::get_string;
  using base_type::get_map;

private:
  // constant reference, no assignment
  this_type& operator=(this_type const&) /* = delete */;
};

// --------------------------------------------------------------------------

/// reference to mutable variant list value
class variant_list_ref : public variant_list_cref
{
  friend class variant_ref;
  typedef variant_list_cref base_type;
  typedef variant_list_ref this_type;

protected:
  explicit variant_list_ref(impl* i = NULL)
    : base_type(i)
  {
  }

public:
  this_type operator=(this_type const&);
  this_type operator=(base_type const&);
  /// exchange contents with another list value
  void swap(this_type&);

  /// clear list elements
  this_type clear();

  /** @name (mutable) element access by index */
  //@{
  using base_type::operator[];
  reference operator[](size_type index);

  using base_type::at;
  reference at(size_type index) { return (*this)[index]; }
  //@}

  /// capacity of underlying storage
  size_type capacity() const;
  /// reserve space in underlying storage
  this_type reserve(size_type);

  /** @name push new elements to the end of the list */
  //@{

  /// append value obtained from a constant variant reference
  this_type push_back(const_reference v);
  /// append arbitrary variant_traits enabled value
  template<typename T>
  this_type push_back(const T& v, SYSX_VARIANT_ENABLE_IF_TRAITS_(T));

  //@}

  // TODO: add iterator interface
};

inline variant_list_ref
variant_list_ref::operator=(this_type const& that)
{
  return *this = base_type(that);
}

inline variant_list_ref
variant_list_ref::operator=(base_type const& that)
{
  variant_ref v(pimpl_);
  v = that;
  return *this;
}

inline variant_list_ref::reference variant_list_ref::operator[](size_type index)
{
  return reference(base_type::operator[](index).pimpl_);
}

inline variant_list_ref
variant_ref::get_list()
{
  return variant_list_ref(base_type::get_list().pimpl_);
}

// --------------------------------------------------------------------------

/// reference to constant variant map
class variant_map_cref : public variant_cref
{
  friend class variant_cref;
  typedef variant_cref base_type;
  typedef variant_map_cref this_type;

protected:
  explicit variant_map_cref(impl* i = NULL)
    : base_type(i)
  {
  }

public:
  typedef size_t size_type;
  typedef variant_cref const_reference;
  typedef variant_ref reference;

  /** @name map queries */
  //@{
  bool empty() const { return size() == 0; }
  size_type size() const;
  //@}

  /** @name map element queries
   * Check for the existence of an entry with a given key
   */
  //@{
  bool has_entry(const char* key) const
  {
    return NULL != do_lookup(key, std::strlen(key), /* allow_fail = */ true);
  }
  bool has_entry(variant_string_cref key) const
  {
    return NULL !=
           do_lookup(key.c_str(), key.length(), /* allow_fail = */ true);
  }
  bool has_entry(std::string const& key) const
  {
    return NULL !=
           do_lookup(key.c_str(), key.length(), /* allow_fail = */ true);
  }
  //@}

  /** @name map element access
   * Accessing an entry with a given key
   */
  //@{
  const_reference operator[](const char* key) const
  {
    return const_reference(do_lookup(key, std::strlen(key)));
  }
  const_reference operator[](variant_string_cref key) const
  {
    return const_reference(do_lookup(key.c_str(), key.length()));
  }
  const_reference operator[](std::string const& key) const
  {
    return const_reference(do_lookup(key.c_str(), key.length()));
  }
  ///@}

  // TODO: add iterator interface

protected:
  impl* do_lookup(const char* key,
                  size_type keylen,
                  bool allow_fail = false) const;

private:
  // exclude non-map value functions
  using base_type::get_bool;
  using base_type::get_int;
  using base_type::get_uint;
  using base_type::get_int64;
  using base_type::get_uint64;
  using base_type::get_double;
  using base_type::get_string;
  using base_type::get_list;

private:
  // constant reference, no assignment
  this_type& operator=(this_type const&) /* = delete */;
};

// --------------------------------------------------------------------------

/// reference to mutable variant map
class variant_map_ref : public variant_map_cref
{
  friend class variant_ref;
  typedef variant_map_cref base_type;
  typedef variant_map_ref this_type;

protected:
  explicit variant_map_ref(impl* i = NULL)
    : base_type(i)
  {
  }

public:
  this_type operator=(base_type const&);
  this_type operator=(this_type const&);
  void swap(this_type&);

  this_type clear();

  /** @name map element access */
  ///@{
  using base_type::operator[];
  reference operator[](const char* key)
  {
    return reference(do_lookup(key, std::strlen(key)));
  }
  reference operator[](std::string const& key)
  {
    return reference(do_lookup(key.c_str(), key.length()));
  }
  ///@}

  ///@name map element addition
  ///@{
  /// add value obtained from a constant variant reference
  this_type push_entry(const char* key, const_reference const& value);
  /// add value obtained from a constant variant reference
  this_type push_entry(std::string const& key, const_reference const& value)
  {
    return push_entry(key.c_str(), value);
  }

  /// add an arbitrary variant_traits enabled value
  template<typename T>
  this_type push_entry(const char* key,
                       const T& value,
                       SYSX_VARIANT_ENABLE_IF_TRAITS_(T));
  /// add an arbitrary variant_traits enabled value
  template<typename T>
  this_type push_entry(std::string const& key,
                       const T& value,
                       SYSX_VARIANT_ENABLE_IF_TRAITS_(T))
  {
    return push_entry<T>(key.c_str(), value);
  }
  ///@}

  // TODO: add iterator interface
};

inline variant_map_ref
variant_map_ref::operator=(this_type const& that)
{
  return *this = base_type(that);
}

inline variant_map_ref
variant_map_ref::operator=(base_type const& that)
{
  variant_ref v(pimpl_);
  v = that;
  return *this;
}

inline variant_map_ref
variant_ref::get_map()
{
  return variant_map_ref(base_type::get_map().pimpl_);
}

// --------------------------------------------------------------------------

/**
 * @brief generic variant type
 *
 * This class provides the first-class type for representing arbitrary values.
 * It can represent its values via a set of primitive types:
 *  * @c null (default)
 *  * boolean values (@c true, @c false)
 *  * numeric values (signed/unsigned integral or floating-point numbers)
 *  * strings
 *  * lists of values
 *  * maps of (key, value) pairs
 *
 * First-class objects of this class have strict value semantics, i.e. each
 * value represents a distinct object.  Due to the hierarchical nature of the
 * data structure, values embedded somewhere in a list or map are referenced
 * by dedicated reference objects (variant_cref, variant_ref, and their
 * specialized variants for strings, lists and maps), with or without constness.
 *
 * Users can add automatic conversions from/to variant objects by providing
 * an implementation (or specialisation) of the variant_traits class.
 * Corresponding specializations for the builtin types, the SystemC data types
 * and some freuquently used standard types are provided by default already.
 *
 * \see variant_list, variant_map, variant_traits
 */
class variant : public variant_ref
{
  typedef variant this_type;

public:
  /// reference to a constant value
  typedef variant_cref const_reference;
  /// reference to a mutable value
  typedef variant_ref reference;
  /// reference to a constant string value
  typedef variant_string_cref const_string_reference;
  /// reference to a mutable string value
  typedef variant_string_ref string_reference;
  /// reference to a constant list value
  typedef variant_list_cref const_list_reference;
  /// reference to a mutable list value
  typedef variant_list_ref list_reference;
  /// reference to a constant map value
  typedef variant_map_cref const_map_reference;
  /// reference to a mutable map value
  typedef variant_map_ref map_reference;

  /// default constructor
  variant()
    : variant_ref()
    , own_pimpl_()
  {
  }

  /// constructor from basic type
  explicit variant(variant_category); ///< @todo drop this?

  /// constructor from arbitrary variant_traits enabled value
  template<typename T>
  explicit variant(T const& src, SYSX_VARIANT_ENABLE_IF_TRAITS_(T));

  variant(this_type const& that);
  variant(const_reference that);

  this_type& operator=(this_type const&);
  this_type& operator=(const_reference);

  void swap(variant& that);
  void swap(reference that)
  {
    init();
    reference::swap(that);
  }

  ~variant();

  /** @name set value functions
   * \see variant_ref
   */
  //@{
  /// set to arbitrary variant_traits enabled value
  template<typename T>
  reference set(T const& v, SYSX_VARIANT_ENABLE_IF_TRAITS_(T))
  {
    init();
    return reference::set(v);
  }

  /// set boolean value
  reference set_bool(bool v)
  {
    init();
    return reference::set_bool(v);
  }

  /// set integer value
  reference set_int(int v)
  {
    init();
    return reference::set_int(v);
  }
  /// set unsigned integer value
  reference set_uint(unsigned v)
  {
    init();
    return reference::set_uint(v);
  }
  /// set 64-bit integer value
  reference set_int64(int64 v)
  {
    init();
    return reference::set_int64(v);
  }
  /// set unsigned 64-bit integer value
  reference set_uint64(uint64 v)
  {
    init();
    return reference::set_uint64(v);
  }

  /// set floating-point value
  reference set_double(double v)
  {
    init();
    return variant_ref::set_double(v);
  }

  string_reference set_string(const char* s)
  {
    init();
    return reference::set_string(s);
  }
  string_reference set_string(const_string_reference s)
  {
    init();
    return reference::set_string(s);
  }
  string_reference set_string(const std::string& s)
  {
    init();
    return reference::set_string(s);
  }

  list_reference set_list()
  {
    init();
    return variant_ref::set_list();
  }

  map_reference set_map()
  {
    init();
    return variant_ref::set_map();
  }
  //@}

  /** @name JSON (de)serialization
   */
  using const_reference::json_serialize;
  bool json_deserialize(std::string const& src)
  {
    init();
    return reference::json_deserialize(src);
  }

  static variant from_json(std::string const& json);
  static std::string to_json(const_reference v);
  //@}

private:
  impl* init();
  impl* do_init();

  impl* own_pimpl_;
};

template<typename T>
variant::variant(T const& v, SYSX_VARIANT_TRAITS_ENABLED_(T))
  : variant_ref()
  , own_pimpl_()
{
  do_init();
  set(v);
}

inline variant::variant(this_type const& that)
  : reference()
  , own_pimpl_()
{
  *this = that;
}

inline variant::variant(const_reference that)
  : reference()
  , own_pimpl_()
{
  *this = that;
}

inline variant&
variant::operator=(this_type const& that)
{
  return operator=(const_reference(that));
}

inline variant::impl*
variant::init()
{
  if (!pimpl_)
    pimpl_ = do_init();
  return pimpl_;
}

inline variant
variant::from_json(std::string const& json)
{
  variant v;
  bool ok = v.json_deserialize(json);
  (void)ok;
  sc_assert(ok);
  return v;
}

inline std::string
variant::to_json(const_reference v)
{
  std::string json;
  bool ok = v.json_serialize(json);
  (void)ok;
  sc_assert(ok);
  return json;
}

///@todo drop me later?
inline std::ostream&
operator<<(std::ostream& os, variant::const_reference v)
{
  os << variant::to_json(v);
  return os;
}

// --------------------------------------------------------------------------

/**
 * @brief list of variant values
 *
 * This class is equivalent to a variant after calling @c set_list().
 * @see variant, variant_list_cref, variant_list_ref
 */
class variant_list : public variant_list_ref
{
  typedef variant_list this_type;

public:
  typedef variant_list_cref const_reference;
  typedef variant_list_ref reference;

  variant_list();

  variant_list(this_type const&);
  variant_list(const_reference);

  this_type& operator=(this_type const&);
  this_type& operator=(const_reference);
  void swap(this_type&);
  void swap(reference that) { reference::swap(that); }

  ~variant_list();

private:
  impl* do_init();
  impl* own_pimpl_;
};

inline variant_list::variant_list()
  : reference()
  , own_pimpl_()
{
  do_init();
}

inline variant_list::variant_list(this_type const& that)
  : reference()
  , own_pimpl_()
{
  do_init();
  *this = that;
}

inline variant_list::variant_list(const_reference that)
  : reference()
  , own_pimpl_()
{
  do_init();
  *this = that;
}

inline variant_list&
variant_list::operator=(this_type const& that)
{
  return operator=(const_reference(that));
}

// --------------------------------------------------------------------------

/**
 * @brief map of (key, variant) pairs
 *
 * This class is equivalent to a variant after calling @c set_map().
 * @see variant, variant_map_cref, variant_map_ref
 */
class variant_map : public variant_map_ref
{
  typedef variant_map this_type;

public:
  typedef variant_map_cref const_reference;
  typedef variant_map_ref reference;

  variant_map();

  variant_map(this_type const&);
  variant_map(const_reference);

  this_type& operator=(this_type const&);
  this_type& operator=(const_reference);
  void swap(this_type&);
  void swap(reference that) { reference::swap(that); }

  ~variant_map();

private:
  impl* do_init();
  impl* own_pimpl_;
};

inline variant_map::variant_map()
  : reference()
  , own_pimpl_()
{
  do_init();
}

inline variant_map::variant_map(this_type const& that)
  : reference()
  , own_pimpl_()
{
  do_init();
  *this = that;
}

inline variant_map::variant_map(const_reference that)
  : reference()
  , own_pimpl_()
{
  do_init();
  *this = that;
}

inline variant_map&
variant_map::operator=(this_type const& that)
{
  return operator=(const_reference(that));
}

// --------------------------------------------------------------------------
// Additional definitions

template<typename T>
variant_list_ref::this_type
variant_list_ref::push_back(const T& value, SYSX_VARIANT_TRAITS_ENABLED_(T))
{
  variant v(value);
  return push_back(const_reference(v));
}

template<typename T>
variant_map_ref
variant_map_ref::push_entry(const char* key,
                            const T& value,
                            SYSX_VARIANT_TRAITS_ENABLED_(T))
{
  variant v(value);
  return push_entry(key, const_reference(v));
}

} /* namespace utils */
} /* namespace sysx */

#undef SYSX_VARIANT_TRAITS_
#undef SYSX_VARIANT_TRAITS_ENABLED_
#undef SYSX_VARIANT_ENABLE_IF_TRAITS_

#endif // SYSX_UTILS_VARIANT_H_INCLUDED_
/* Taf!
 * :tag: (variant,h)
 */
