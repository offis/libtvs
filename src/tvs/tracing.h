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

#ifndef TVS_H_INCLUDED_
#define TVS_H_INCLUDED_

/**
 * \defgroup tracing Tracing
 * \brief stream-based tracing library
 */

/**
 * \file  tracing.h
 * \brief main header file for the tracing
 * \ingroup tracing
 */

/**
 * \namespace tracing
 * \brief main tracing namespace
 * \ingroup tracing
 */
namespace tracing {
}

#include "tvs/tracing/timed_duration.h"
#include "tvs/tracing/timed_sequence.h"
#include "tvs/tracing/timed_stream.h"
#include "tvs/tracing/timed_value.h"

#include "tvs/tracing/timed_variant.h"

#include "tvs/tracing/timed_reader.h"
#include "tvs/tracing/timed_writer.h"

// FIXME: cleanup includes
#include "tvs/tracing/timed_stream.tpp"
#include "tvs/tracing/timed_stream_processor_base.h"

#include "tvs/tracing/timed_stream_traits.h"

#endif /* TVS_H_INCLUDED_ */
/* Taf!
 * :tag: (tracing,h)
 */
