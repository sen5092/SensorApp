// NetworkConstants.hpp
#pragma once
#include <cstdint>

namespace NetworkConstants {
    inline constexpr std::uint16_t MIN_PORT = 0;
    inline constexpr std::uint16_t MAX_PORT = 65535;
}

inline bool isValidPortRange(int32_t port) {
    return port > NetworkConstants::MIN_PORT && port <= NetworkConstants::MAX_PORT;
}