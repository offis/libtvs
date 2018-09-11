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
 * @file   variant.cpp
 * @author Philipp A. Hartmann, OFFIS
 */

#include "tvs/utils/variant.h"
#include "tvs/utils/rapidjson.h"

#include <algorithm> // std::swap
#include <mutex>
#include <vector> // std::vector

namespace sysx {
namespace utils {

typedef rapidjson::CrtAllocator allocator_type;
typedef rapidjson::UTF8<> encoding_type;
typedef rapidjson::GenericValue<encoding_type, allocator_type> json_value;
typedef rapidjson::GenericDocument<encoding_type, allocator_type> json_document;

static allocator_type json_allocator;

// wrapper implementation is simply a RapidJSON value
typedef json_value impl_type;
// helper to convert to wrapper implementation
static inline impl_type*
impl_cast(void* p)
{
  return static_cast<impl_type*>(p);
}

// ----------------------------------------------------------------------------
// variant_cache

/// internal helper to pool explicitly allocated RapidJSON value objects
struct variant_pool
{
  typedef std::vector<impl_type*> free_list;

  static impl_type* create() { return instance().do_create(); }

  static void free(impl_type* obj) { instance().do_free(obj); }

  ~variant_pool()
  {
    for (auto it = free_.begin(), end = free_.end(); it != end; ++it)
      delete *it;
  }

private:
  variant_pool() = default;
  static variant_pool& instance()
  {
    static variant_pool instance_;
    return instance_;
  }

  impl_type* do_create()
  {
    std::lock_guard<std::mutex> l(mut_);
    if (free_.empty())
      return new impl_type();

    impl_type* ret = free_.back();
    free_.pop_back();
    return ret;
  }

  void do_free(impl_type* obj)
  {
    std::lock_guard<std::mutex> l(mut_);
    free_.push_back(obj);
  }

  std::mutex mut_;

  free_list free_;
};

#define PIMPL(x) (impl_cast((x).pimpl_))

#define DEREF(x) (*PIMPL(x))

#define THIS (PIMPL(*this))

#define VALUE_ASSERT(Cond, Msg)                                                \
  do {                                                                         \
    if (!(Cond))                                                               \
      SYSX_REPORT_ERROR(report::variant_error)                                 \
        << Msg << " (condition: " #Cond ")";                                   \
  } while (false)

// ----------------------------------------------------------------------------
// variant_cref

bool
operator==(variant_cref const& left, variant_cref const& right)
{
  if (PIMPL(left) == PIMPL(right))
    return true;

  if (!PIMPL(left) || !PIMPL(right))
    return false;

  return DEREF(left) == DEREF(right);
}

variant_category
variant_cref::category() const
{
  if (!THIS)
    return VARIANT_NULL;

  switch (THIS->GetType()) {
    case rapidjson::kFalseType:
    case rapidjson::kTrueType:
      return VARIANT_BOOL;

    case rapidjson::kNumberType:
      return VARIANT_NUMBER;

    case rapidjson::kStringType:
      return VARIANT_STRING;

    case rapidjson::kArrayType:
      return VARIANT_LIST;

    case rapidjson::kObjectType:
      return VARIANT_MAP;

    case rapidjson::kNullType:
    default:
      return VARIANT_NULL;
  }
}

bool
variant_cref::is_null() const
{
  return !THIS || THIS->IsNull();
}

bool
variant_cref::is_bool() const
{
  return THIS && THIS->IsBool();
}

bool
variant_cref::is_int() const
{
  return THIS && THIS->IsInt();
}

bool
variant_cref::is_int64() const
{
  return THIS && THIS->IsInt64();
}

bool
variant_cref::is_uint() const
{
  return THIS && THIS->IsUint();
}

bool
variant_cref::is_uint64() const
{
  return THIS && THIS->IsUint64();
}

bool
variant_cref::is_double() const
{
  return THIS && THIS->IsDouble();
}

bool
variant_cref::is_string() const
{
  return THIS && THIS->IsString();
}

bool
variant_cref::is_list() const
{
  return THIS && THIS->IsArray();
}

bool
variant_cref::is_map() const
{
  return THIS && THIS->IsObject();
}

bool
variant_cref::get_bool() const
{
  (is_bool());
  return THIS->GetBool();
}

#define ASSERT_TYPE(Cond) VALUE_ASSERT(Cond, "invalid type access")

int
variant_cref::get_int() const
{
  ASSERT_TYPE(is_int());
  return THIS->GetInt();
}

unsigned
variant_cref::get_uint() const
{
  ASSERT_TYPE(is_uint());
  return THIS->GetUint();
}

int64
variant_cref::get_int64() const
{
  ASSERT_TYPE(is_int64());
  return THIS->GetInt64();
}

uint64
variant_cref::get_uint64() const
{
  ASSERT_TYPE(is_uint64());
  return THIS->GetUint64();
}

double
variant_cref::get_double() const
{
  ASSERT_TYPE(is_number());
  return THIS->GetDouble();
}

variant_string_cref
variant_cref::get_string() const
{
  ASSERT_TYPE(is_string());
  return variant_string_cref(pimpl_);
}

variant_list_cref
variant_cref::get_list() const
{
  ASSERT_TYPE(is_list());
  return variant_list_cref(pimpl_);
}

variant_map_cref
variant_cref::get_map() const
{
  ASSERT_TYPE(is_map());
  return variant_map_cref(pimpl_);
}

// ----------------------------------------------------------------------------
// variant_ref

void
variant_ref::swap(variant_ref& that)
{
  VALUE_ASSERT(pimpl_ && that.pimpl_, "swap with invalid value failed");
  THIS->Swap(DEREF(that));
}

variant_ref
variant_ref::operator=(variant_cref const& that)
{
  if (that.is_null())
    set_null();
  else {
    SYSX_ASSERT(THIS);
    THIS->CopyFrom(DEREF(that), json_allocator);
  }
  return *this;
}

variant_ref
variant_ref::set_null()
{
  if (THIS)
    THIS->SetNull();
  return this_type(THIS);
}

variant_ref
variant_ref::set_bool(bool v)
{
  SYSX_ASSERT(THIS);
  THIS->SetBool(v);
  return this_type(THIS);
}

variant_ref
variant_ref::set_int(int v)
{
  SYSX_ASSERT(THIS);
  THIS->SetInt(v);
  return this_type(THIS);
}

variant_ref
variant_ref::set_uint(unsigned v)
{
  SYSX_ASSERT(THIS);
  THIS->SetUint(v);
  return this_type(THIS);
}

variant_ref
variant_ref::set_int64(int64 v)
{
  SYSX_ASSERT(THIS);
  THIS->SetInt64(v);
  return this_type(THIS);
}

variant_ref
variant_ref::set_uint64(uint64 v)
{
  SYSX_ASSERT(THIS);
  THIS->SetUint64(v);
  return this_type(THIS);
}

variant_ref
variant_ref::set_double(double d)
{
  SYSX_ASSERT(THIS);
  THIS->SetDouble(d);
  return this_type(THIS);
}

variant_string_ref
variant_ref::set_string(const char* s, size_t len)
{
  SYSX_ASSERT(THIS);
  THIS->SetString(s, len, json_allocator);
  return variant_string_ref(THIS);
}

variant_list_ref
variant_ref::set_list()
{
  SYSX_ASSERT(THIS);
  THIS->SetArray();
  return variant_list_ref(THIS);
}

variant_map_ref
variant_ref::set_map()
{
  SYSX_ASSERT(THIS);
  THIS->SetObject();
  return variant_map_ref(THIS);
}

// ----------------------------------------------------------------------------
// variant_string_cref

variant_string_cref::size_type
variant_string_cref::size() const
{
  return THIS->GetStringLength();
}

const char*
variant_string_cref::c_str() const
{
  return THIS->GetString();
}

bool
operator==(variant_string_cref const& left, const char* right)
{
  return !right ? false : DEREF(left) == right;
}
bool
operator==(variant_string_cref const& left, std::string const& right)
{
  return DEREF(left) == rapidjson::StringRef(right.c_str(), right.size());
}
bool
operator==(const char* left, variant_string_cref const& right)
{
  return !left ? false : DEREF(right) == left;
}
bool
operator==(std::string const& left, variant_string_cref const& right)
{
  return DEREF(right) == rapidjson::StringRef(left.c_str(), left.size());
}

// ----------------------------------------------------------------------------
// variant_string_ref

void
variant_string_ref::swap(this_type& that)
{
  VALUE_ASSERT(pimpl_ && that.pimpl_, "swap with invalid value failed");
  THIS->Swap(DEREF(that));
}

// ----------------------------------------------------------------------------
// variant_list_cref

variant_list_cref::size_type
variant_list_cref::size() const
{
  return THIS->Size();
}

variant_cref variant_list_cref::operator[](size_type index) const
{
  return variant_cref(&(*THIS)[index]);
}

// ----------------------------------------------------------------------------
// variant_list_ref

void
variant_list_ref::swap(this_type& that)
{
  VALUE_ASSERT(pimpl_ && that.pimpl_, "swap with invalid value failed");
  THIS->Swap(DEREF(that));
}

variant_list_ref
variant_list_ref::clear()
{
  THIS->Clear();
  return *this;
}

variant_list_ref::size_type
variant_list_ref::capacity() const
{
  return THIS->Capacity();
}

variant_list_ref
variant_list_ref::reserve(size_type new_capacity)
{
  THIS->Reserve(new_capacity, json_allocator);
  return *this;
}

variant_list_ref
variant_list_ref::push_back(const_reference value)
{
  json_value v;
  if (PIMPL(value))
    v.CopyFrom(DEREF(value), json_allocator);
  THIS->PushBack(v, json_allocator);
  return *this;
}

// ----------------------------------------------------------------------------
// variant_map_cref

variant_map_cref::size_type
variant_map_cref::size() const
{
  return THIS->MemberCount();
}

variant_cref::impl*
variant_map_cref::do_lookup(const char* key,
                            size_type keylen,
                            bool allow_fail /* = false */) const
{
  json_value kv(rapidjson::StringRef(key, keylen));
  json_value::ConstMemberIterator it = THIS->FindMember(kv);
  if (it == THIS->MemberEnd()) {
    if (allow_fail)
      return nullptr;

    SYSX_REPORT_ERROR(report::variant_error)
      << "variant map has no element with key '" << key << "'";
    return nullptr;
  }
  return const_cast<json_value*>(&it->value);
}

// ----------------------------------------------------------------------------
// variant_map_ref

void
variant_map_ref::swap(this_type& that)
{
  VALUE_ASSERT(pimpl_ && that.pimpl_, "swap with invalid value failed");
  THIS->Swap(DEREF(that));
}

variant_map_ref
variant_map_ref::clear()
{
  THIS->RemoveAllMembers();
  return *this;
}

variant_map_ref
variant_map_ref::push_entry(const char* key,
                            variant::const_reference const& value)
{
  json_value k(key, json_allocator);
  json_value v;
  if (PIMPL(value))
    v.CopyFrom(DEREF(value), json_allocator);
  THIS->AddMember(k, v, json_allocator);
  return *this;
}

// ----------------------------------------------------------------------------
// variant(, _list, _map ) -- owning wrapper implementations

variant::variant(variant_category cat)
  : variant_ref()
  , own_pimpl_()
{
  do_init();
  switch (cat) {
    case VARIANT_BOOL:
      THIS->SetBool(false);
      break;
    case VARIANT_NUMBER:
      THIS->SetUint(0u);
      break;
    case VARIANT_STRING:
      THIS->SetString("");
      break;
    case VARIANT_LIST:
      THIS->SetObject();
      break;
    case VARIANT_MAP:
      THIS->SetObject();
      break;
    case VARIANT_NULL:
    default
      :
      /* do nothing */;
  }
}

#define WRAPPER_ASSIGN_PRECOND_(Kind) WRAPPER_ASSIGN_PRECOND_FOR_##Kind
#define WRAPPER_ASSIGN_PRECOND_FOR_variant                                     \
  if (that.is_null()) {                                                        \
    set_null();                                                                \
    return *this;                                                              \
  }                                                                            \
  init()
#define WRAPPER_ASSIGN_PRECOND_FOR_variant_list                                \
  SYSX_ASSERT(is_list() && that.is_list())
#define WRAPPER_ASSIGN_PRECOND_FOR_variant_map                                 \
  SYSX_ASSERT(is_map() && that.is_map())

#define WRAPPER_DO_INIT_(Kind) WRAPPER_DO_INIT_##Kind
#define WRAPPER_DO_INIT_variant ((void)0)
#define WRAPPER_DO_INIT_variant_list (THIS)->SetArray()
#define WRAPPER_DO_INIT_variant_map (THIS)->SetObject()

#define DEFINE_WRAPPER_(Kind)                                                  \
  Kind::this_type& Kind::operator=(const_reference that)                       \
  {                                                                            \
    WRAPPER_ASSIGN_PRECOND_(Kind);                                             \
    reference::operator=(that);                                                \
    return *this;                                                              \
  }                                                                            \
                                                                               \
  void Kind::swap(this_type& that)                                             \
  {                                                                            \
    using std::swap;                                                           \
    swap(pimpl_, that.pimpl_);                                                 \
    swap(own_pimpl_, that.own_pimpl_);                                         \
  }                                                                            \
                                                                               \
  Kind::impl* Kind::do_init()                                                  \
  {                                                                            \
    SYSX_ASSERT(!own_pimpl_);                                                    \
    pimpl_ = own_pimpl_ = variant_pool::create();                              \
    WRAPPER_DO_INIT_(Kind);                                                    \
    return THIS;                                                               \
  }                                                                            \
                                                                               \
  Kind::~Kind() { variant_pool::free(impl_cast(own_pimpl_)); }

DEFINE_WRAPPER_(variant)
DEFINE_WRAPPER_(variant_list)
DEFINE_WRAPPER_(variant_map)

// ----------------------------------------------------------------------------
// JSON (de)serialize

bool
variant_cref::json_serialize(std::string& dst) const
{
  rapidjson::StringBuffer buf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
  if (!THIS) {
    writer.Null();
  } else {
    THIS->Accept(writer);
  }
  VALUE_ASSERT(writer.IsComplete(), "incomplete JSON sequence");
  dst.assign(buf.GetString(), buf.GetSize());
  return true;
}

bool
variant_ref::json_deserialize(std::string const& src)
{
  json_document doc;
  try {
    doc.Parse(src.c_str());
  } catch (rapidjson::ParseException const& e) {
    SYSX_REPORT_ERROR(report::variant_error)
      << "JSON parsing failed: " << GetParseError_En(e.Code())
      << "\n"
         "\t'"
      << src << "' (offset: " << e.Offset() << ")";
    return false;
  }
  THIS->Swap(doc);
  return true;
}

} /* namespace utils */
} /* namespace sysx */
/* Taf!
 * :tag: (variant,s)
 */
