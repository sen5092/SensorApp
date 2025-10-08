// NetworkConstants.hpp
#pragma once
#include <cstdint>

namespace NetworkConstants {
    inline constexpr std::uint16_t kMinPort = 0;
    inline constexpr std::uint16_t kMaxPort = 65535;
}

inline bool isValidPortRange(int32_t port) {
    return port > NetworkConstants::kMinPort && port <= NetworkConstants::kMaxPort;
}