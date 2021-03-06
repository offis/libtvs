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
 * \file   timed_stream_processor_base.h
 * \author Philipp Ittershagen <philipp.ittershagen@offis.de>
 * \brief  timed stream processor convenience base class
 * \see    timed_stream.h timed_value.h
 */

#ifndef TVS_TIMED_STREAM_PROCESSOR_BASE_H_INCLUDED_
#define TVS_TIMED_STREAM_PROCESSOR_BASE_H_INCLUDED_

#include <tvs/tracing/timed_object.h>
#include <tvs/tracing/timed_reader_base.h>
#include <tvs/tracing/timed_stream_base.h>
#include <tvs/tracing/timed_writer_base.h>

#include <memory>
#include <unordered_set>

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
  : public timed_base
  , public timed_listener_if
{
  using base_type = timed_object;
  using this_type = timed_stream_processor_base;

  using stream_base_type = timed_stream_base;
  using reader_base_type = timed_reader_base;
  using writer_base_type = timed_writer_base;

  using reader_ptr_type = std::shared_ptr<timed_reader_base>;
  using writer_ptr_type = std::shared_ptr<timed_writer_base>;

  using reader_collection_type = std::vector<reader_ptr_type>;
  using writer_collection_type = std::vector<writer_ptr_type>;

  using duration_type = stream_base_type::duration_type;

  /// Add an input stream of a writer to this processor.
  ///
  /// \returns a shared_ptr to the the internally created reader which is
  /// attached to the stream of the given reader
  template<typename T, typename Traits>
  std::shared_ptr<tracing::timed_reader<T, Traits>>
  in(timed_writer<T, Traits>&);

  /// Add an input stream to this processor.
  ///
  /// \returns a shared_ptr to the the internally created reader which is
  /// attached to the given stream
  template<typename T, typename Traits>
  std::shared_ptr<tracing::timed_reader<T, Traits>>
  in(timed_stream<T, Traits>&);

  /// Add an output stream of a reader to this processor.
  ///
  /// \returns a shared_ptr to the the internally created writer which is
  /// attached to the stream of the given reader
  template<typename T, typename Traits>
  std::shared_ptr<tracing::timed_writer<T, Traits>>
  out(timed_reader<T, Traits>&);

  /// Add an output stream to this processor.
  ///
  /// \returns a shared_ptr to the the internally created writer which is
  /// attached to the given stream
  template<typename T, typename Traits>
  std::shared_ptr<tracing::timed_writer<T, Traits>>
  out(timed_stream<T, Traits>&);

protected:
  timed_stream_processor_base();

  /// Returns a reference to the container of all attached input readers.
  reader_collection_type& inputs() { return inputs_; }
  reader_collection_type const& inputs() const { return inputs_; }

  /// Returns a reference to the attached output writers.
  writer_collection_type& outputs() { return outputs_; }
  writer_collection_type const& outputs() const { return outputs_; }

  /// User-defined implementation for updating the state of this processor.
  ///
  /// This method is called when all input streams have tuples available.  The
  /// duration given is the minimum duration of the first tuple on all streams,
  /// i.e. calling reader->front(dur) is guaranteed to succeed.  The
  /// implementation should return the consumed time which is then used to
  /// advance this processors's time via commit(). The user is responsible for
  /// consuming the tuples on the input streams.
  virtual duration_type process(duration_type dur) { return dur; }

  /// Advance the (distributed) local time of this processor instance.
  ///
  /// This method is called by the timed_base class when synchronising the local
  /// time via commit().  This default implementation commits the output stream
  /// for the given duration.
  ///
  /// \param duration the duration which will consumed by the input stream and
  ///                 produced at the output stream
  /// \returns the committed duration
  duration_type do_commit(duration_type) override;

  void do_add_input(reader_ptr_type&&);
  void do_add_output(writer_ptr_type&&);

  ~timed_stream_processor_base() override = default;

private:
  /// Checks if a minimum token duration is available on all input streams and
  /// then calls process().
  virtual void notify(reader_base_type&) override;

  void update_cache();

  reader_collection_type inputs_;
  writer_collection_type outputs_;

  std::unordered_set<timed_reader_base const*> available_inputs_;
  duration_type front_duration_{ duration_type::infinity() };
};

template<typename T, typename Traits>
std::shared_ptr<tracing::timed_reader<T, Traits>>
timed_stream_processor_base::in(tracing::timed_writer<T, Traits>& writer)
{
  using stream_type =
    typename std::remove_reference<decltype(writer)>::type::stream_type;

  auto stream = dynamic_cast<stream_type*>(&writer.stream());
  SYSX_ASSERT(stream != nullptr);
  return this->in(*stream);
}

template<typename T, typename Traits>
std::shared_ptr<tracing::timed_reader<T, Traits>>
timed_stream_processor_base::in(tracing::timed_stream<T, Traits>& stream)
{
  using reader_type =
    typename std::remove_reference<decltype(stream)>::type::reader_type;

  std::stringstream name;
  name << stream.basename() << "_reader";

  auto ret = std::make_shared<reader_type>(
    host::gen_unique_name(name.str().c_str()), stream);

  this->do_add_input(ret);
  return ret;
}

template<typename T, typename Traits>
std::shared_ptr<tracing::timed_writer<T, Traits>>
timed_stream_processor_base::out(tracing::timed_reader<T, Traits>& reader)
{
  using stream_type =
    typename std::remove_reference<decltype(reader)>::type::stream_type;

  auto stream = dynamic_cast<stream_type*>(&reader.stream());
  SYSX_ASSERT(stream != nullptr);
  return this->out(*stream);
}

template<typename T, typename Traits>
std::shared_ptr<tracing::timed_writer<T, Traits>>
timed_stream_processor_base::out(tracing::timed_stream<T, Traits>& stream)
{
  using writer_type =
    typename std::remove_reference<decltype(stream)>::type::writer_type;
  auto ret = std::make_shared<writer_type>(stream);
  this->do_add_output(ret);
  return ret;
}

/// Policy-based processor template.
///
/// \tparam T the value_type of the timed_value
/// \tparam ProcessingPolicy the Policy on how to process the available tokens
///         of the input reader(s)
template<typename T, template<class> class ProcessingPolicy>
class timed_stream_processor
  : public timed_stream_processor_base
  , protected ProcessingPolicy<T>
{

  using policy_type = ProcessingPolicy<T>;
  using base_type = timed_stream_processor_base;

public:
  timed_stream_processor() = default;

protected:
  // apply policy_type::process() on all input readers.
  duration_type process(duration_type dur) final
  {
    for (auto&& in : inputs()) {
      policy_type::process(*in, dur);
    }
    return dur;
  }
};

} // namespace tracing

#endif /* TVS_TIMED_STREAM_PROCESSOR_BASE_H_INCLUDED_ */
