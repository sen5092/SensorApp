// NetworkConstants.hpp
#pragma once
#include <cstdint>

namespace NetworkConstants {
    inline constexpr std::uint16_t MinPort = 0;
    inline constexpr std::uint16_t MaxPort = 65535;
}

inline bool isValidPortRange(int32_t port) {
    return port > NetworkConstants::MinPort && port <= NetworkConstants::MaxPort;
}