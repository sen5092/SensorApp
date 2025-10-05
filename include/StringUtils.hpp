/**
 * @file StringUtils.hpp
 * @brief Small string utilities used across the SensorApp project.
 *
 * This header provides simple, header-only helpers for case conversion
 * and case-insensitive comparison. The functions are lightweight and
 * suitable for use in tests and non-performance-critical code paths.
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <string>

namespace StringUtils {

inline std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char cha){ return static_cast<char>(std::tolower(cha)); });
    return str;
}

inline std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char cha){ return static_cast<char>(std::toupper(cha)); });
    return str;
}

inline bool iequals(const std::string& aStr, const std::string& bStr) {
    return std::equal(aStr.begin(), aStr.end(),
                      bStr.begin(), bStr.end(),
                      [](unsigned char ch1, unsigned char ch2) {
                          return std::tolower(ch1) == std::tolower(ch2);
                      });
}

} // namespace StringUtils
