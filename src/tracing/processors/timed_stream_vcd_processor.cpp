
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
                                                       std::ostream& out)
  : named_object(modscope)
  , out_(out)
{}

timed_stream_vcd_processor::~timed_stream_vcd_processor()
{
  print_timestamp(this->local_time());
  out_ << "$vcdclose " << this->local_time() << " $end\n";
}

std::string
timed_stream_vcd_processor::next_identifier()
{
  // ASCII start/end characters for encoding the wire ID
  char const first_id = '!';
  char const last_id = '~';
  int const range = last_id - first_id + 1;

  auto next = vcd_id_++;

  // produce an identifier in the range [first_id, last_id] with the necessary
  // amount of character symbols
  std::string id{};
  do {
    int rem = next % range;
    id.insert(id.begin(), rem + first_id);
    next /= range;
  } while (next != 0);

  return id;
}

void
timed_stream_vcd_processor::print_timestamp(time_type const& stamp)
{
  using sysx::units::sc_time_cast;
  out_ << "#"
       << static_cast<uint64_t>(sc_time_cast<sysx::units::time_type>(stamp) /
                                scale_)
       << "\n";
}

void
timed_stream_vcd_processor::write_header()
{
  out_ << "$timescale " << sysx::units::engineering_prefix << scale_
       << " $end\n";

  out_ << "$scope module " << this->name() << " $end\n";

  for (const auto& vcd : this->vcd_streams_) {
    if (vcd->scope() != std::string("")) {
      out_ << "$scope module " << vcd->scope() << " $end\n";
      vcd->print_node_information(out_);
      out_ << "$upscope $end\n";
    } else {
      vcd->print_node_information(out_);
    }
  }

  out_ << "$upscope $end\n";

  out_ << "$enddefinitions $end\n"
       << "$dumpvars\n";

  for (auto&& vcd : this->vcd_streams_) {
    vcd->print_default_value(out_);
  }

  out_ << "$end\n";
}

void
timed_stream_vcd_processor::notify(reader_base_type&)
{

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
    while (rd.available() && rd.local_time() <= until) {
      if (vcd->value_changed()) {
        vcd->print_front_value(temp_sstr_);
        ordered.insert(std::make_pair(rd.local_time(), temp_sstr_.str()));
        temp_sstr_.str("");
        vcd->update_value();
      }
      rd.pop();
    }
  }

  // print the ordered tuples and the time stamp if necessary
  time_type stamp = duration_type::infinity();
  for (auto&& o : ordered) {
    if (stamp != o.first) {
      print_timestamp(o.first);
      stamp = o.first;
    }
    out_ << o.second;
  }

  commit(until);
}

} // namespace tracing
