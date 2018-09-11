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

#ifndef SYSX_UTILS_TYPE_ID_H_INCLUDED_
#define SYSX_UTILS_TYPE_ID_H_INCLUDED_

#include <string>
#include <typeinfo>

#include <tvs/utils/assert.h>
#include <tvs/utils/report.h>

namespace sysx {
namespace utils {

class type_id
{
public:
  typedef const std::type_info* type_ptr;
  typedef type_id this_type;

  template<typename T>
  explicit type_id(T const& t)
    : ptr_(&typeid(t))
  {
    SYSX_ASSERT(ptr_ && "No RTTI information found!");
  }

  std::type_info const& info() const { return *ptr_; }

  std::string name() const;

  bool operator==(this_type const& rhs) const { return *ptr_ == *(rhs.ptr_); }

  bool operator!=(this_type const& rhs) const { return *ptr_ != *(rhs.ptr_); }

  bool operator<(this_type const& rhs) const
  {
    return ptr_->before(*(rhs.ptr_));
  }

  bool operator>(this_type const& rhs) const { return rhs.ptr_->before(*ptr_); }

  bool operator>=(this_type const& rhs) const { return !(*this < rhs); }

  bool operator<=(this_type const& rhs) const { return !(*this > rhs); }

  template<typename T>
  static type_id of()
  {
    return type_id(typeid(T));
  }

  static const type_id none;

private:
  type_id()
    : ptr_(&typeid(void))
  {}

  explicit type_id(std::type_info const& t)
    : ptr_(&t)
  {}

private:
  type_ptr ptr_;
};

std::ostream&
operator<<(std::ostream&, type_id);

} // namespace utils
} // namespace sysx

#endif /* SYSX_UTILS_TYPE_ID_H_INCLUDED_ */
/* Taf!
 * :tag: (utils,h)
 */
