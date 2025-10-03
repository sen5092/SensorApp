// StringUtils.hpp
#pragma once

#include <algorithm>
#include <cctype>
#include <string>

namespace StringUtils {

inline std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

inline std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return s;
}

inline bool iequals(const std::string& a, const std::string& b) {
    return std::equal(a.begin(), a.end(),
                      b.begin(), b.end(),
                      [](unsigned char c1, unsigned char c2) {
                          return std::tolower(c1) == std::tolower(c2);
                      });
}

} // namespace StringUtils
