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

#ifndef TVS_TIMED_STREAM_PROCESSOR_BASE_H_INCLUDED_
#define TVS_TIMED_STREAM_PROCESSOR_BASE_H_INCLUDED_

#include "tvs/tracing/timed_object.h"
#include "tvs/tracing/timed_reader_base.h"
#include "tvs/tracing/timed_stream_base.h"
#include "tvs/tracing/timed_writer_base.h"

#include "tvs/utils/unique_ptr.h"

namespace tracing {

template<typename, typename>
class timed_reader;
template<typename, typename>
class timed_writer;
template<typename, typename>
class timed_stream;

/// Timed-value stream processor base class for N inputs and one output.
///
/// This class provides the infrastructure to combine several different streams
/// and merge them or consume their contents othwise (e.g. by printting to a
/// file or stdout).  The overall combination can be provided by implemeting the
/// \a process class member.
///
/// This stream processor listens for incoming values on all input streams.
/// When _all_ input streams have tuples available, the processor will call the
/// \a process() member function with the minimum available duration on all
/// streams.
///
struct timed_stream_processor_base
  : public timed_object
  , public timed_listener_if
{
  using base_type = timed_object;
  using this_type = timed_stream_processor_base;

  using stream_base_type = timed_stream_base;
  using reader_base_type = timed_reader_base;
  using writer_base_type = timed_writer_base;

  using reader_ptr_type = std::unique_ptr<timed_reader_base>;
  using writer_ptr_type = std::unique_ptr<timed_writer_base>;

  using reader_collection_type = std::vector<reader_ptr_type>;
  using writer_collection_type = std::vector<writer_ptr_type>;

  using duration_type = stream_base_type::duration_type;

  /// Add an input stream of a writer to this processor.
  template<typename T, typename Traits>
  void in(timed_writer<T, Traits>&);

  /// Add an input stream to this processor.
  template<typename T, typename Traits>
  void in(timed_stream<T, Traits>&);

  /// Add an output stream of a reader to this processor.
  template<typename T, typename Traits>
  void out(timed_reader<T, Traits>&);

  /// Add an output stream to this processor.
  template<typename T, typename Traits>
  void out(timed_stream<T, Traits>&);

protected:
  timed_stream_processor_base(const char*);

  /// Returns a reference to the container of all attached input readers.
  reader_collection_type& inputs() { return inputs_; }

  /// Returns a reference to the attached output writers.
  writer_collection_type& outputs() { return outputs_; }

  /// User-defined implementation for updating the state of this processor.
  ///
  /// This method is called when the given duration is available on all input
  /// streams.  It should return the consumed time which is then used to advance
  /// this processors's time via commit().  The user is responsible for
  /// consuming the tuples on the input streams.
  virtual duration_type process(duration_type) = 0;

  /// Advance the (distributed) local time of this processor instance.
  ///
  /// This method is called by the timed_base class when synchronising the local
  /// time via commit().  This default implementation commits the output stream
  /// for the given duration.
  ///
  /// \param duration the duration which will consumed by the input stream and
  ///                 produced at the output stream
  /// \returns the committed duration
  virtual duration_type do_commit(duration_type) override;

  void do_add_input(reader_ptr_type&&);
  void do_add_output(writer_ptr_type&&);

  ~timed_stream_processor_base() = default;

private:
  /// Checks if a minimum token duration is available on all input streams and
  /// then calls process().
  void notify(reader_base_type&) override final;

  reader_collection_type inputs_;
  writer_collection_type outputs_;
};

template<typename T, typename Traits>
void
timed_stream_processor_base::in(tracing::timed_writer<T, Traits>& writer)
{
  using stream_type =
    typename std::remove_reference<decltype(writer)>::type::stream_type;

  auto stream = dynamic_cast<stream_type*>(&writer.stream());
  SYSX_ASSERT(stream != nullptr);
  this->in(*stream);
}

template<typename T, typename Traits>
void
timed_stream_processor_base::in(tracing::timed_stream<T, Traits>& stream)
{
  using reader_type =
    typename std::remove_reference<decltype(stream)>::type::reader_type;

  std::stringstream name;
  name << stream.basename() << "_reader";

  this->do_add_input(detail::make_unique<reader_type>(
    host::gen_unique_name(name.str().c_str()), stream));
}

template<typename T, typename Traits>
void
timed_stream_processor_base::out(tracing::timed_reader<T, Traits>& reader)
{
  using stream_type =
    typename std::remove_reference<decltype(reader)>::type::stream_type;

  auto stream = dynamic_cast<stream_type*>(&reader.stream());
  SYSX_ASSERT(stream != nullptr);
  this->out(*stream);
}

template<typename T, typename Traits>
void
timed_stream_processor_base::out(tracing::timed_stream<T, Traits>& stream)
{
  using writer_type =
    typename std::remove_reference<decltype(stream)>::type::writer_type;
  this->do_add_output(detail::make_unique<writer_type>(stream));
}

/// Policy-based processor template.
///
/// \tparam T the value_type of the timed_value
/// \tparam ProcessingPolicy the Policy on how to process the available tokens
///         of the input reader(s)
template<typename T, template<class> typename ProcessingPolicy>
class timed_stream_processor
  : public timed_stream_processor_base
  , protected ProcessingPolicy<T>
{

  using policy_type = ProcessingPolicy<T>;
  using base_type = timed_stream_processor_base;

public:
  timed_stream_processor(char const* name)
    : base_type(name)
  {}

protected:
  // apply policy_type::process() on all input readers.
  virtual duration_type process(duration_type dur) override final
  {
    for (auto&& in : inputs()) {
      policy_type::process(*in, dur);
      in->pop_duration(dur);
    }
    return dur;
  }
};

} // namespace tracing

#endif /* TVS_TIMED_STREAM_PROCESSOR_BASE_H_INCLUDED_ */
