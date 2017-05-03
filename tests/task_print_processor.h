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

#ifndef TASK_PRINT_PROCESSOR_INCLUDED_
#define TASK_PRINT_PROCESSOR_INCLUDED_

#include "tvs/tracing.h"

#include <ostream>
#include <sstream>

/**
 * \brief Simple stream sink for printing values to an output stream.
 *
 * \tparam T The type of the timed_value
 * \tparam Traits The traits type of the stream
 */
template<typename T,
         template<typename> class Traits = tracing::timed_state_traits>
struct task_print_processor : tracing::timed_stream_processor_base<T, Traits>
{
  using this_type = task_print_processor;
  using base_type = tracing::timed_stream_processor_base<T, Traits>;
  using reader_type = typename base_type::reader_type;

  task_print_processor(char const* name)
    : base_type(name)
  {
  }

  friend std::ostream& operator<<(std::ostream& out, this_type const& t)
  {
    t.print(out);
    return out;
  }

private:
  /// Print out the buffered stringstream contents.
  void print(std::ostream& out) const { out << output_.str(); }

  /// Consume all available reader tokens by adding them to the output
  /// stringstream.
  void do_notify(reader_type& reader) override
  {
    // clear the stream before inserting any new values
    output_.clear();
    output_.str(std::string());

    while (!reader.empty()) {
      output_ << reader.local_time() << ":" << reader.front() << std::endl;
      reader.pop();
    }
  }

  std::stringstream output_;
};

#endif // TASK_PRINT_PROCESSOR_INCLUDED_
