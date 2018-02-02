
#include "tvs/tracing/processors/timed_stream_vcd_processor.h"

#include "tvs/tracing/processors/timed_stream_processor_base.h"

#include "tvs/utils/assert.h"
#include "tvs/utils/report.h"

#include "tvs/tracing/report_msgs.h"

#include <map>

namespace tracing {

timed_stream_vcd_processor::timed_stream_vcd_processor(char const* name,
                                                       std::ostream& out,
                                                       char vcd_start_signal)
  : base_type(name)
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
    out_ << "$scope module " << this->name() << " $end\n";
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
timed_stream_vcd_processor::process(duration_type dur)
{
  using sysx::units::sc_time_cast;
  using sysx::units::time_type;

  if (!header_written_) {
    write_header();
    header_written_ = true;
  }

  unsigned long stamp = sc_time_cast<time_type>(this->local_time()) / scale_;
  out_ << "#" << stamp << "\n";

  // FIXME: avoid splitting?
  for (auto&& vcd : this->vcd_streams_) {
    vcd->print_value(out_, dur);
    vcd->reader().pop_duration(dur);
  }

  return dur;
}

} // namespace tracing
