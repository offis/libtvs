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
 * \file   timed_annotation.h
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>
 * \brief  timed annotations for block-based tracing
 */

#ifndef TVS_TIMED_ANNOTATION_H
#define TVS_TIMED_ANNOTATION_H

#include <tvs/tracing/timed_duration.h>

#include <vector>

namespace tracing {
namespace impl {

struct var_push_if
{
  virtual void push_duration(timed_duration const&) = 0;

protected:
  virtual ~var_push_if() = default;
};

class timed_annotation
{

public:
  using this_type = timed_annotation;

  timed_annotation(timed_duration const& duration)
    : duration_(duration)
  {
    // set up scope
    scope = this;
  }

  static void register_var(impl::var_push_if* ptr)
  {
    if (scope != nullptr) {
      scope->do_register_var(ptr);
    }
  }

  ~timed_annotation()
  {
    // TODO: handle nested scopes?
    SYSX_ASSERT(scope == this);
    scope->push();
    scope = nullptr;
  }

  // allow usage in 'if' statements
  operator bool() const { return false; }

private:
  void do_register_var(impl::var_push_if* ptr)
  {
    auto it = std::find(timed_vars_.begin(), timed_vars_.end(), ptr);
    // register only once
    if (it == timed_vars_.end())
      timed_vars_.push_back(ptr);
  }

  void push() const
  {
    for (auto&& v : timed_vars_) {
      v->push_duration(duration_);
    }
  }

  timed_duration duration_;
  std::vector<impl::var_push_if*> timed_vars_;

  static this_type* scope;
};

template<typename T>
struct block_wrapper_t
{

  template<typename... Args>
  block_wrapper_t(Args&&... args)
    : obj_{ std::forward<Args>(args)... }
  {
  }

private:
  T obj_;
};

// clang-format off
#define TVS_TIMED_BLOCK(duration)                           \
  if (auto SYSX_IMPL_CONCAT_(tvs_timed_block_, __LINE__) =  \
      ::tracing::impl::timed_annotation{ duration }) {} else
// clang-format on

} // namespace impl
} // namespace tracing

#endif /* TVS_TIMED_ANNOTATION_H */
