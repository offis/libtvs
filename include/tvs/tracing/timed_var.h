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
 * \file   timed_var.h
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>
 * \brief  traceable variable for use in block-based annotations
 */

#ifndef TVS_TIMED_VAR_H
#define TVS_TIMED_VAR_H

#include <tvs/tracing/timed_annotation.h>
#include <tvs/tracing/timed_stream.h>

namespace tracing {
namespace impl {

template<typename T, typename Traits>
class timed_var : protected var_push_if
{
  using this_type = timed_var<T, Traits>;
  using writer_type = timed_writer<T, Traits>;

public:
  timed_var(writer_type& writer)
    : writer_(&writer)
    , var_{}
  {
  }

  template<typename OtherT>
  this_type& operator=(OtherT const& other)
  {
    var_ = other;
    timed_annotation::register_var(this);
    return *this;
  }

private:
  // callback impl
  void push_duration(timed_duration const& dur) override
  {
    writer_->push(var_, dur);
  }

  writer_type* writer_;
  T var_;
};

} // namespace impl

template<typename T, typename Traits>
inline impl::timed_var<T, Traits>
timed_var(timed_writer<T, Traits>& writer)
{
  return impl::timed_var<T, Traits>(writer);
}

} // namespace tracing

#endif /* TVS_TIMED_VAR_H */
