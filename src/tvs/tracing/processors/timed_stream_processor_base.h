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

#include "tvs/utils/unique_ptr.h"

namespace tracing {

class timed_writer_base;

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
struct timed_stream_processor_base : public timed_object,
                                     public timed_listener_if
{
  using base_type = timed_object;
  using this_type = timed_stream_processor_base;

  using stream_base_type = timed_stream_base;
  using reader_base_type = timed_reader_base;
  using writer_base_type = timed_writer_base;

  using reader_ptr_type = std::unique_ptr<timed_reader_base>;
  using writer_ptr_type = std::unique_ptr<timed_writer_base>;

  using reader_sequence_type = std::vector<reader_ptr_type>;

  using duration_type = stream_base_type::duration_type;

  /// Add an input stream of a writer to this processor.
  template <typename T, typename Policy>
  void in(tracing::timed_writer<T, Policy>& writer);

  /// Add an input stream to this processor.
  template <typename T, typename Policy>
  void in(tracing::timed_stream<T, Policy>& stream);

  /// Add an output stream of a reader to this processor.
  template <typename T, typename Policy>
  void out(tracing::timed_reader<T, Policy>& reader);

  /// Add an output stream to this processor.
  template <typename T, typename Policy>
  void out(tracing::timed_stream<T, Policy>& stream);

protected:
  timed_stream_processor_base(const char*);

  /// Returns a reference to the container of all attached input readers.
  reader_sequence_type& inputs() { return inputs_; }

  /// Returns a reference to the output writer.
  writer_base_type& output();

  /// User-defined implementation for updating the state of this processor.
  ///
  /// This method is called when the given duration is available on all input
  /// streams.  It should return the consumed time which is then used to advance
  /// this processors's time via commit().
  virtual duration_type process(duration_type) = 0;

  /// Advance the (distributed) local time of this processor instance.
  ///
  /// This method is called by the timed_base class when synchronising the local
  /// time via commit().  The default behaviour of this implementation is to pop
  /// all tuples for the given duration from all input streams and call commit
  /// on the output stream.
  ///
  /// \param duration the duration which will consumed by the input stream and
  ///                 produced at the output stream
  /// \returns the committed duration
  virtual duration_type do_commit(duration_type) override;

  void do_add_input(reader_ptr_type&&);
  void do_add_output(writer_ptr_type&&);

private:
  /// Checks if a minimum token duration is available on all input streams and
  /// then calls process().
  void notify(reader_base_type&) override final;

  reader_sequence_type inputs_;
  writer_ptr_type output_;
};

template <typename T, typename Policy>
void
timed_stream_processor_base::in(tracing::timed_writer<T, Policy>& writer)
{
  using stream_type = tracing::timed_stream<T, Policy>;
  auto stream = dynamic_cast<stream_type*>(&writer.stream());
  SYSX_ASSERT(stream != nullptr);
  this->in(*stream);
}

template <typename T, typename Policy>
void
timed_stream_processor_base::in(tracing::timed_stream<T, Policy>& stream)
{
  using reader_type = tracing::timed_reader<T, Policy>;

  std::cout << "processor_base::in() for stream " << stream.basename() << "\n";

  std::stringstream name;
  name << stream.basename() << "_in";

  this->do_add_input(std::make_unique<reader_type>(
    stream_base_type::gen_unique_name(name.str().c_str()), stream));
}

template <typename T, typename Policy>
void
timed_stream_processor_base::out(tracing::timed_reader<T, Policy>& reader)
{
  using stream_type = tracing::timed_stream<T, Policy>;
  auto stream = dynamic_cast<stream_type*>(&reader.stream());
  SYSX_ASSERT(stream != nullptr);
  this->out(*stream);
}

template <typename T, typename Policy>
void
timed_stream_processor_base::out(tracing::timed_stream<T, Policy>& stream)
{
  using writer_type = tracing::timed_writer<T, Policy>;
  this->do_add_output(std::make_unique<writer_type>(stream));
}

} // namespace tracing

#endif /* TVS_TIMED_STREAM_PROCESSOR_BASE_H_INCLUDED_ */
