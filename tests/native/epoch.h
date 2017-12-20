#ifndef TVS_NATIVE_EPOCH_H
#define TVS_NATIVE_EPOCH_H

#include <chrono>

std::chrono::time_point<std::chrono::high_resolution_clock>
get_epoch();

#endif /* TVS_NATIVE_EPOCH_H */
