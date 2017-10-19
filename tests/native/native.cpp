
#include "producer.h"

#include "gtest/gtest.h"

#include "../print_processor.h"

#include <systemc>

int
sc_main(int argc, char* argv[])
{

  using printer_type = ::test_printer<producer::events>;

  printer_type printer("my_printer", printer_type::OUTPUT_COUT);

  producer prod;

  printer.in(prod.writer_);

  prod.run();

  return 0;
}
