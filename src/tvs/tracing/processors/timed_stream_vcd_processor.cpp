
#include "tvs/tracing/processors/timed_stream_vcd_processor.h"

#include "tvs/tracing/processors/timed_stream_processor_base.h"

#include "tvs/utils/assert.h"
#include "tvs/utils/report.h"

#include "tvs/tracing/report_msgs.h"

#include <map>

namespace tracing {

timed_stream_vcd_processor::timed_stream_vcd_processor(char const* modscope,
                                                       std::ostream& out,
                                                       char vcd_start_signal)
  : named_object(modscope)
  , out_(out)
  , vcd_id_(vcd_start_signal)
  , header_written_(false)
  , scale_(1.0 * sysx::si::picoseconds)
{}

timed_stream_vcd_processor::~timed_stream_vcd_processor() {}

void
timed_stream_vcd_processor::write_header()
{
  out_ << "$timescale " << sysx::units::engineering_prefix << scale_
       << " $end\n";

  for (const auto& vcd : this->vcd_streams_) {
    out_ << "$scope module " << vcd->scope() << " $end\n";
    vcd->header_defn(out_);
    out_ << "$upscope $end\n";
  }

  out_ << "$enddefinitions $end\n"
       << "$dumpvars\n";

  for (auto&& vcd : this->vcd_streams_) {
    vcd->default_value(out_);
  }

  out_ << "$end\n";
}

timed_stream_vcd_processor::duration_type
timed_stream_vcd_processor::process(duration_type /* unused */)
{

  if (!header_written_) {
    write_header();
    header_written_ = true;
  }

  unsigned long scaled =
    sysx::units::sc_time_cast<sysx::units::time_type>(this->local_time()) /
    scale_;

  out_ << "#" << scaled << "\n";

  // figure out the next local time to advance to
  std::vector<time_type> next;

  // write the first tuple of all synchronised streams
  for (auto&& vcd : this->vcd_streams_) {
    auto& rd = vcd->reader();
    if (rd.local_time() == this->local_time()) {
      vcd->print_front_value(out_);
      rd.pop();
    }
    next.push_back(rd.local_time());
  }

  return *std::min_element(next.begin(), next.end()) - local_time();
}

} // namespace tracing
