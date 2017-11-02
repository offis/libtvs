
#include "tvs/tracing/processors/timed_stream_vcd_processor.h"

#include "tvs/tracing/processors/timed_stream_processor_base.h"

#include "tvs/utils/assert.h"
#include "tvs/utils/report.h"

#include "tvs/tracing/report_msgs.h"

#include <map>

namespace tracing {

struct timed_stream_vcd_processor::timed_stream_vcd_impl
{

private:
  struct vcd_reader;

public:
  using reader_base_type = tracing::timed_reader_base;

  using reader_container_type = std::vector<vcd_reader>;

  explicit timed_stream_vcd_impl(tracing::timed_stream_vcd_processor* base,
                                 char vcd_start_signal)
    : signal_(vcd_start_signal)
  {}

  void add_vcd(reader_base_type* reader,
               const char* name,
               std::string type,
               unsigned int bitwidth)
  {
    this->vcd_readers_.emplace_back(reader, type, bitwidth, signal_++, name);
  }

  reader_container_type const& readers() const { return vcd_readers_; }

  char reader_id(reader_base_type* reader) const
  {

    // find the corresponding vcd ref to get the VCD stream id
    auto vcd_ref = std::find_if(
      vcd_readers_.begin(), vcd_readers_.end(), [&](vcd_reader const& val) {
        return val.reader() == reader;
      });

    SYSX_ASSERT(vcd_ref != vcd_readers_.end() &&
                "Cannot find VCD ID for the reader!");

    return vcd_ref->id();
  }

private:
  /// Wrapper for a timed_reader_base containing additional VCD-related metadata
  struct vcd_reader
  {
    using this_type = vcd_reader;

    vcd_reader(reader_base_type* reader,
               std::string type,
               unsigned int bitwidth,
               char id,
               char const* name)
      : reader_(reader)
      , type(type)
      , bitwidth(bitwidth)
      , id_(id)
      , name(name)
    {}

    char id() const { return id_; }

    std::string defn() const
    {
      std::stringstream out;
      out << "$var " << type << " " << bitwidth << " " << id_ << " " << name
          << " $end";
      return out.str();
    }

    reader_base_type const* reader() const { return reader_; }

  private:
    reader_base_type* reader_;
    std::string type;
    unsigned int bitwidth;
    char id_;
    char const* name;
  };

  char signal_;
  std::vector<vcd_reader> vcd_readers_;
};

timed_stream_vcd_processor::timed_stream_vcd_processor(char const* name,
                                                       std::ostream& out,
                                                       char vcd_start_signal)
  : base_type(name)
  , pimpl_(detail::make_unique<timed_stream_vcd_impl>(this, vcd_start_signal))
  , out_(out)
  , header_written_(false)
{}

timed_stream_vcd_processor::~timed_stream_vcd_processor() {}

void
timed_stream_vcd_processor::write_header()
{
  sysx::units::time_type scale = 1.0 * sysx::si::picoseconds;

  out_ << "$timescale " << sysx::units::engineering_prefix << scale << " $end\n"
       << "$scope module " << this->name() << " $end\n";

  for (const auto& vcd : this->pimpl_->readers()) {
    out_ << vcd.defn() << "\n";
  }

  out_ << "$upscope $end\n"
       << "$enddefinitions $end\n";
}

void
timed_stream_vcd_processor::add_vcd(reader_base_type* reader,
                                    const char* name,
                                    std::string type,
                                    unsigned int bitwidth)
{
  if (header_written_) {
    SYSX_REPORT_ERROR(report::header_written) % this->name()
      << "while adding reader " << reader->name();
  }
  pimpl_->add_vcd(reader, name, type, bitwidth);
}

timed_stream_vcd_processor::duration_type
timed_stream_vcd_processor::process(duration_type dur)
{
  if (!header_written_) {
    write_header();
    header_written_ = true;
  }

  // we first store the available tuples of all readers in the sorted multimap
  std::multimap<tracing::time_type, std::string> value_sequence;

  for (auto&& reader : this->inputs()) {

    char stream_id = pimpl_->reader_id(reader.get());

    // consume everything (including zero-time tuples) using timed_variant
    auto tuple = reader->front_variant(dur);

    std::stringstream val_rep;
    val_rep << tuple.value() << stream_id;
    value_sequence.insert(
      std::make_pair(this->local_time() + tuple.duration(), val_rep.str()));
    reader->pop();
  }

  // now print the multimap (sorted by time) and update the time stamp in the
  // VCD stream if necessary
  tracing::time_type stamp;
  for (auto&& val : value_sequence) {
    // print new timestamp into VCD stream if necessary
    if (stamp != val.first)
      out_ << "#" << val.first.value() << "\n";

    out_ << val.second << "\n";
    stamp = val.first;
  }
  return dur;
}

} // namespace tracing
