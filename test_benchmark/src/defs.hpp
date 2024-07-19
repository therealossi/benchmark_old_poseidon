
#ifndef defs_hpp_
#define defs_hpp_

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <mutex>

constexpr uint64_t UNKNOWN = std::numeric_limits<uint64_t>::max();

constexpr uint32_t UNKNOWN_CODE = std::numeric_limits<uint32_t>::max();

#define PF_PAGE_SIZE           1048576 // 1024 * 1024 = 1 MB
#endif
