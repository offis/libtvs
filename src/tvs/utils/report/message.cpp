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

#include "tvs/utils/report/message.h"

#include "tvs/utils/assert.h"
#include "tvs/utils/macros.h"
#include "tvs/utils/report.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace sysx {
namespace report {

/* ----------------------- implementation class ------------------------ */

class message::impl
{
public:
  class element;
  class substring;
  class replacer;

  explicit impl(const char*);
  stream_type& inject();
  stream_type& append();
  std::string combine() const;
  ~impl();

private:
  typedef std::vector<element*> snippets_type;
  typedef std::vector<replacer*> replacers_type;

  snippets_type snippets_;
  replacers_type replacers_;
  replacers_type::iterator current_;
  std::stringstream tail_;

  static const char replacer_pattern[];
  static const char replacer_unknown[];
};

const char message::impl::replacer_pattern[] = "%s";
const char message::impl::replacer_unknown[] = "<unknown>";

/* -------------------------- con/destruction -------------------------- */

message::message(const char* msg)
  : impl_(new impl(msg))
{}

message::~message()
{
  delete impl_;
}

/* --------------------------- forward calls --------------------------- */

message::stream_type&
message::inject()
{
  return impl_->inject();
}

message::stream_type&
message::append()
{
  return impl_->append();
}

std::string
message::combine() const
{
  return impl_->combine();
}

/* --------------------------- implementation -------------------------- */

class message::impl::element
{
public:
  virtual void print(message::stream_type&) const = 0;
  virtual ~element() = default;

}; // message::impl::element

class message::impl::substring : public message::impl::element
{
public:
  substring(const std::string& msg)
    : msg_(msg)
  {}

  substring(const std::string& msg,
            std::string::size_type from,
            std::string::size_type to)
    : msg_(msg, from, to - from)
  {}

  void print(message::stream_type& out) const override { out << msg_; }

private:
  const std::string msg_;
}; // message::impl::substring

class message::impl::replacer : public message::impl::element
{
public:
  replacer()
    : str_()
  {}

  message::stream_type& stream()
  {
    filled_ = true;
    return str_;
  }

  void print(message::stream_type& out) const override
  {
    /// print replaced value, iff set, replacer_unknown otherwise
    if (filled_) {
      out << str_.str();
    } else {
      out << replacer_unknown;
    }
  }

private:
  std::stringstream str_;
  bool filled_;

}; // message::impl::replacer

message::impl::impl(char const* const msg_tpl)
  : snippets_()
  , replacers_()
  , current_(replacers_.end())
  , tail_()
{
  const size_t pat_sz = sizeof(replacer_pattern) - 1;

  SYSX_ASSERT(msg_tpl);      // ensure valid message
  std::string msg = msg_tpl; // temporary string

  std::string::size_type from = 0;
  std::string::size_type to = 0;
  std::string::size_type last = msg.size();

  while (from < last) {

    // look for next replacer/substring
    to = msg.find(replacer_pattern, from);
    bool pat_found = (to != std::string::npos);

    if (!pat_found) { // no other pattern
      to = last;      // get remaining string
    }

    // store substring, if present
    if (to - from) {
      snippets_.push_back(new substring(msg, from, to));
    }
    // next search start
    from = to;

    // we found a pattern, store pattern replacer
    if (pat_found) {
      auto* rpl = new replacer();
      snippets_.push_back(rpl);
      replacers_.push_back(rpl); // remember replacer for inject()
      from += pat_sz;            // skip placeholder
    }
  }
  // reseat beginning position
  current_ = !replacers_.empty() ? replacers_.begin() : replacers_.end();
}

/* anonymous */ namespace {

/// small helper struct to delete a pointer
struct delete_helper
{
  template<typename Type>
  void operator()(const Type* ptr)
  {
    delete ptr;
  }
};

/// small helper struct to print from a pointer to a std::ostream
struct print_helper
{
  explicit print_helper(std::ostream& out)
    : out_(out)
  {}

  template<typename Type>
  void operator()(const Type* const ptr)
  {
    ptr->print(out_);
  }

private:
  std::ostream& out_;
};

} // anonymous namespace

std::string
message::impl::combine() const
{
  std::stringstream result;
  std::for_each(snippets_.begin(), snippets_.end(), print_helper(result));
  result << tail_.str();
  return result.str();
}

message::stream_type&
message::impl::inject()
{
  if (sysx_unlikely(current_ == replacers_.end())) {
    SYSX_REPORT_FATAL(sysx::report::internal_error) % "Invalid use of replacer"
      << "\n  Intermediate report message: " << combine();
    std::terminate();
  }

  // return current replacer stream
  stream_type& result = (*current_)->stream();
  // and increment marker
  ++current_;
  return result;
}

message::stream_type&
message::impl::append()
{
  return tail_;
}

message::impl::~impl()
{
  std::for_each(snippets_.begin(), snippets_.end(), delete_helper());
}

} // namespace report
} // namespace sysx

/* Taf!
 * :tag: (utils,s)
 */
