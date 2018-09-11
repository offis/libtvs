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
 * \file   timed_writer.h
 * \author Philipp A. Hartmann <pah@computer.org>
 * \brief  timed value writer for tracing (implementation)
 * \see    timed_stream.h
 */

#ifndef TVS_TIMED_WRITER_BASE_H_INCLUDED_
#define TVS_TIMED_WRITER_BASE_H_INCLUDED_

#include <tvs/tracing/timed_object.h>
#include <tvs/tracing/timed_stream_base.h>

#include <tvs/tracing/timed_variant.h>

#include <memory>

namespace tracing {

enum writer_mode
{
  STREAM_ATTACH = 0x1,
  STREAM_CREATE = 0x2,
  STREAM_AUTO = STREAM_ATTACH | STREAM_CREATE,
  STREAM_DEFAULT = STREAM_ATTACH
};

class timed_writer_base : public timed_base
{
  friend class timed_stream_base;
  typedef timed_stream_base stream_type;

public:
  const char* name() { return stream_->name(); }

  virtual stream_type& stream() { return *stream_; }
  virtual stream_type const& stream() const { return *stream_; }

  time_type begin_time() const { return stream_->local_time(); }
  time_type end_time() const { return stream_->end_time(); }
  duration_type duration() const { return stream_->duration(); }

  //! commit/sync interface
  //!{
  void commit() { stream_->commit(); }
  void commit(time_type const& until) { stream_->commit(until); }
  void commit(duration_type const& dur) { stream_->commit(dur); }

  time_type sync(duration_type const& dur) { return stream_->sync(dur); }
  //!}

  virtual void push_variant(timed_variant const&) = 0;

  ~timed_writer_base() override;

  timed_writer_base(timed_writer_base&& other)
    : own_stream_{ std::move(other.own_stream_) }
  {
    this->re_attach(other);
  }

  timed_writer_base& operator=(timed_writer_base&& other)
  {
    this->own_stream_ = std::move(other.own_stream_);
    this->re_attach(other);
    return *this;
  }

  timed_writer_base(timed_writer_base const&) = delete;
  timed_writer_base& operator=(timed_writer_base const&) = delete;

protected:
  explicit timed_writer_base(stream_type* own_stream = nullptr);

  void attach(const char* name);
  void attach(timed_stream_base& stream);
  void detach();

  void check_stream(const char* context);

private:
  void re_attach(timed_writer_base& other)
  {
    if (other.stream_ != nullptr) {
      auto str = other.stream_;
      other.detach();
      this->attach(*str);
    }
  }

  stream_type* stream_{};
  std::unique_ptr<stream_type> own_stream_;
};

} // namespace tracing

#endif /* TVS_TIMED_WRITER_BASE_H_INCLUDED_ */
/* Taf!
 */
