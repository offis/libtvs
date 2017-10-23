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
 * \file   object_provider.h
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>
 * \brief  Generic object provider interface
 */

#ifndef TVS_OBJECT_HOST_H_INCLUDED_
#define TVS_OBJECT_HOST_H_INCLUDED_

#include "tvs/tracing/timed_duration.h"

#include <functional>

namespace tracing {

class timed_object;
class timed_stream_base;

struct object_host
{
  using cb_type = std::function<bool(timed_stream_base*)>;
  using sync_fn_type = std::function<void(time_type const&)>;

  static void for_each_stream_in_scope(cb_type);

  /// Synchronise with the simulation/implementation model time and the
  /// configured sync function.
  static void sync_with_model(time_type until);
};

/// Register a synchronisation function to be called for synchronising the
/// time with a model/implememtation.  By default, we use
/// sc_core::wait(time_type).
void
register_sync(object_host::sync_fn_type fn);

} // namespace tracing

#endif // TVS_OBJECT_HOST_H_INCLUDED_
