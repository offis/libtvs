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
 * \file   timed_stream_processor_base.h
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>
 * \brief  timed stream processor convenience base class
 * \see    timed_stream.h timed_value.h
 */

#ifndef TVS_TIMED_STREAM_PROCESSOR_H_INCLUDED_
#define TVS_TIMED_STREAM_PROCESSOR_H_INCLUDED_

#include "tvs/tracing/timed_reader_base.h"
#include "tvs/tracing/timed_stream_base.h"

#include "tvs/tracing/timed_object.h"
#include "tvs/utils/unique_ptr.h"

#include "tvs/utils/assert.h"

namespace tracing {

// forward declarations
template<typename, template<typename> class>
class timed_writer;
template<typename, template<typename> class>
class timed_reader;
template<typename>
struct timed_state_traits;

/**
 * \brief A generic stream processor base class for stream sinks.
 *
 * This class can be used to model a stream sink with multiple streams
 * connected.  Each stream is assigned a newly created reader.  Whenever a new
 * commit window is available, the base class calls the virtual method \a
 * do_notify with the corresponding reader for consuming the stream contents.
 *
 * \tparam T The type of the timed_value
 * \tparam Traits The traits type of the stream
 */
template<typename T, template<typename> class Traits = timed_state_traits>
struct timed_stream_processor_base : timed_listener_if
{
  using this_type = timed_stream_processor_base;
  using reader_type = timed_reader<T, Traits>;
  using writer_type = timed_writer<T, Traits>;
  using stream_type = typename reader_type::stream_type;
  using stream_base_type = timed_stream_base;

  using reader_ptr_type = std::unique_ptr<reader_type>;
  using reader_vector_type = std::vector<reader_ptr_type>;

  /// Attach a stream to a locally created reader
  void attach(stream_type& stream)
  {
    auto reader = std::make_unique<reader_type>(name_, stream);
    reader->listen(*this);
    readers_.emplace_back(std::move(reader));
  }

  void attach(stream_base_type& base)
  {
    auto stream = dynamic_cast<stream_type*>(&base);
    SYSX_ASSERT(stream != nullptr);
    attach(*stream);
  }

  /// Attach to stream inside of a writer
  void attach(writer_type& writer) { attach(writer.stream()); }

protected:
  /// Implement this method to get notified when any attached stream has
  /// committed a sequence.
  virtual void do_notify(reader_type&) = 0;

  timed_stream_processor_base(char const* name)
    : name_(name)
  {
  }

private:
  void notify(timed_reader_base& base) override
  {
    reader_type* reader = dynamic_cast<reader_type*>(&base);
    SYSX_ASSERT(reader != nullptr);
    do_notify(*reader);
  }

  reader_vector_type readers_;
  char const* name_;
};

} // namespace tracing

#endif /* TVS_TIMED_STREAM_PROCESSOR_H_INCLUDED_ */
