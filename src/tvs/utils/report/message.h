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

#ifndef SYSX_UTILS_REPORT_MESSAGE_H_INCLUDED_
#define SYSX_UTILS_REPORT_MESSAGE_H_INCLUDED_

#include "tvs/utils/noncopyable.h"

#include <iosfwd>
#include <sstream>
#include <string>

namespace sysx {
namespace report {

/**
 * \class message
 * \brief helper class for sysx::report classes
 *
 * This class is used as an internal wrapper/storage for COMPLEX runtime
 * message.  The class provides a simplistic printf()-like formatting
 * mechanism.
 *
 * \see sysx::report
 *
 */
class message : private sysx::utils::noncopyable
{
  /// real message implementation
  class impl;
  /// pointer to real implementation
  impl* const impl_;

public:
  typedef std::ostream stream_type;

  /// create a message from a template
  explicit message(char const*);
  /// set the next replacer
  stream_type& inject();
  /// append an additional string to the message
  stream_type& append();
  /// combine all parts to a single string
  std::string combine() const;
  /// destructor
  ~message();
};

/// fill the current replacer
template<typename T>
message&
operator%(message& msg, T const& data)
{
  msg.inject() << data;
  return msg;
}

/// append an arbitrary element
template<typename T>
message&
operator<<(message& msg, T const& data)
{
  msg.append() << data;
  return msg;
}

/// support for at least some manipulators (at the end of the message)
inline message&
operator<<(message& msg, std::ostream& (*pfn)(std::ostream&))
{
  // Call manipulator function on apnnd's stream
  pfn(msg.append());
  return msg;
}

} // namespace report
} // namespace sysx

#endif /* SYSX_UTILS_REPORT_MESSAGE_H_INCLUDED_ */
/* Taf!
 * :tag: (utils,h)
 */
