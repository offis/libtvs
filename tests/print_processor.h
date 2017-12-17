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

#ifndef PRINT_PROCESSOR_H_INCLUDED_
#define PRINT_PROCESSOR_H_INCLUDED_

#include "tvs/tracing.h"

#include <ostream>
#include <sstream>

/**
 * \brief Simple printer that buffers the underlying print processor output.
 *
 * \tparam T The type of the timed_value
 * \tparam Traits The traits type of the stream
 */
template<typename T>
struct test_printer : public tracing::timed_stream_print_processor<T>
{
  using this_type = test_printer<T>;
  using base_type = tracing::timed_stream_print_processor<T>;

  enum output_type
  {
    OUTPUT_BUFFERED = 0,
    OUTPUT_COUT,
  };

  test_printer(char const* name, output_type out = OUTPUT_BUFFERED)
    : base_type(name)
    , buf_()
  {
    base_type::set_ostream(out == OUTPUT_COUT ? std::cout : buf_);
  }

  using base_type::in;

  void print(std::ostream& out) const { out << buf_.str(); }

  void clear()
  {
    buf_.clear();
    buf_.str(std::string());
  }

private:
  std::stringstream buf_;
};

#endif // PRINT_PROCESSOR_H_INCLUDED_
