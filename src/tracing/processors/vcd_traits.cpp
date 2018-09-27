
#include "tvs/tracing/processors/vcd_traits.h"

#include <boost/format.hpp>

#include <algorithm>
#include <bitset>
#include <climits>
#include <iomanip>
#include <ostream>

namespace tracing {

#define DEFINE_ATTR_(Type, Attr, Value)                                        \
  template<>                                                                   \
  char const* vcd_traits<Type>::Attr()                                         \
  {                                                                            \
    return Value;                                                              \
  }

#define DEFINE_BITWIDTH_(Type, Value)                                          \
  template<>                                                                   \
  uint16_t vcd_traits<Type>::bitwidth()                                        \
  {                                                                            \
    return Value;                                                              \
  }

#define DEFINE_PRINT_(Type)                                                    \
  template<>                                                                   \
  void vcd_traits<Type>::print(std::ostream& out, value_type const& val)

#define DEFINE_TYPE_TRAITS_(Type, HeaderId, TraceId, BitWidth)                 \
  DEFINE_ATTR_(Type, header_id, HeaderId)                                      \
  DEFINE_ATTR_(Type, trace_id, TraceId)                                        \
  DEFINE_BITWIDTH_(Type, BitWidth)                                             \
  DEFINE_PRINT_(Type)

DEFINE_TYPE_TRAITS_(int, "real", "r", sizeof(int))
{
  out << val;
}

DEFINE_TYPE_TRAITS_(bool, "wire", "b", 1)
{
  out << (val ? "1" : "0");
}


DEFINE_TYPE_TRAITS_(double, "real", "r", sizeof(double))
{
  out << val;
}

DEFINE_TYPE_TRAITS_(std::string, "wire", "b", 800)
{
  for (std::size_t i = 0; i < bitwidth() / 8; i++) {
    char data;
    if (i < val.size()) {
      data = val[i];
    } else {
      data = 0;
    }
    out << std::bitset<8>(data);
  }
}

} // namespace tracing
