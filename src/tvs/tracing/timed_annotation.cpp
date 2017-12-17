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
 * \file   timed_annotation.cpp
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>
 * \brief  timed annotations for block-based tracing
 */

#include "tvs/tracing/timed_annotation.h"

namespace tracing {
namespace impl {

timed_annotation* timed_annotation::scope = nullptr;

} // namespace impl
} // namespace tracing
