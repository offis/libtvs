
#include "tvs/tracing/processors/timed_stream_vcd_processor.h"

#include "tvs/tracing/processors/timed_stream_processor_base.h"

#include "tvs/utils/assert.h"
#include "tvs/utils/report.h"

#include "tvs/tracing/report_msgs.h"

#include <cstdint>
#include <map>

namespace tracing {

char const*
vcd_stream_container_base::scope() const
{

#ifndef SYSX_NO_SYSTEMC
  if (scope_.empty()) {
    auto parent = this->reader().stream().get_parent_object();
    if (parent != nullptr)
      return parent->name();
  }
#endif

  return scope_.c_str();
}

timed_stream_vcd_processor::timed_stream_vcd_processor(char const* modscope,
                                                       std::ostream& out,
                                                       char vcd_start_signal)
  : named_object(modscope)
  , out_(out)
  , vcd_id_(vcd_start_signal)
  , header_written_(false)
  , scale_(1.0 * sysx::si::picoseconds)
{}

timed_stream_vcd_processor::~timed_stream_vcd_processor() = default;

void
timed_stream_vcd_processor::write_header()
{
  out_ << "$timescale " << sysx::units::engineering_prefix << scale_
       << " $end\n";

  out_ << "$scope module " << this->name() << " $end\n";

  for (const auto& vcd : this->vcd_streams_) {
    if (vcd->scope() != std::string("")) {
      out_ << "$scope module " << vcd->scope() << " $end\n";
      vcd->header_defn(out_);
      out_ << "$upscope $end\n";
    } else {
      vcd->header_defn(out_);
    }
  }

  out_ << "$upscope $end\n";

  out_ << "$enddefinitions $end\n"
       << "$dumpvars\n";

  for (auto&& vcd : this->vcd_streams_) {
    vcd->default_value(out_);
  }

  out_ << "$end\n";
}

void
timed_stream_vcd_processor::notify(reader_base_type&)
{
  auto scale_time = [&](time_type const& t) {
    return static_cast<std::uint64_t>(
      sysx::units::sc_time_cast<sysx::units::time_type>(t) / scale_);
  };

  if (!header_written_) {
    write_header();
    header_written_ = true;
  }

  // find the minimum available time of all VCD input streams
  auto it =
    std::min_element(this->inputs().cbegin(),
                     this->inputs().cend(),
                     [](std::shared_ptr<timed_reader_base> const& lhs,
                        std::shared_ptr<timed_reader_base> const& rhs) {
                       return lhs->available_until() < rhs->available_until();
                     });

  time_type until = (*it)->available_until();

  if (until <= local_time()) {
    return;
  }

  // create a map of all tuples to order them
  std::multimap<time_type, std::string> ordered;
  for (auto&& vcd : this->vcd_streams_) {
    auto& rd = vcd->reader();
    // std::cout << rd.count() << " tuples: " << rd;
    while (rd.available() && rd.local_time() <= until) {
      if (vcd->value_changed()) {
        vcd->print_front_value(temp_sstr_);
        ordered.insert(std::make_pair(rd.local_time(), temp_sstr_.str()));
        vcd->update_value();
      }
      rd.pop();
      temp_sstr_.str("");
    }
  }

  // print the ordered tuples
  time_type stamp = duration_type::infinity();
  for (auto&& o : ordered) {
    if (stamp != o.first) {
      out_ << "#" << scale_time(o.first) << "\n";
      stamp = o.first;
    }
    out_ << o.second;
  }

  commit(until);
}

} // namespace tracing
