

#include "epoch.h"

#include <chrono>

std::chrono::time_point<std::chrono::high_resolution_clock>
get_epoch()
{
  using namespace std::chrono;
  using clock_type = high_resolution_clock;

  auto now = clock_type::now();

  // get the nanoseconds in this second

  // tt stores time in seconds since epoch
  std::time_t tt = clock_type::to_time_t(now);

  // broken time as of now
  std::tm bt = *std::localtime(&tt);

  // alter broken time to the beginning of today
  bt.tm_hour = 0;
  bt.tm_min = 0;
  bt.tm_sec = 0;

  // convert broken time back into std::time_t
  tt = std::mktime(&bt);

  // start of today in system_clock units
  return system_clock::from_time_t(tt);

}
