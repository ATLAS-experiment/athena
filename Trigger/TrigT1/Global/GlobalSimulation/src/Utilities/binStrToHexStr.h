/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_BINSTRTOHEXSTR_H
#define GLOBALSIM_BINSTRTOHEXSTR_H

#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>

namespace GlobalSim {

  constexpr char
  hexChar(std::string_view bits){
    unsigned int value{};
    for (const char c : bits) {
      value = (value << 1) | (c == '1');
    }
    // Upper case: the implementations these files are compared against -- GoldenGate
    // and the firmware simulation -- both write upper case hex.
    return (value < 10) ? static_cast<char>('0' + value) : static_cast<char>('A' + value - 10);
  }
  
  inline std::string
  binStrToHexStr(std::string_view s){
    if (s.starts_with("0b") || s.starts_with("0B")) {
      s.remove_prefix(2);
    }
    if (s.empty()) {
      return {};
    }
    if (!std::ranges::all_of(s, [](char c) { return c == '0' || c == '1';})) {
      throw std::invalid_argument("binary string contains non-binary character");
    }
    std::string result;
    result.reserve((s.size() + 3) / 4);
    const auto firstGroupSize = s.size() % 4;
    std::size_t pos{};
    if (firstGroupSize != 0) {
      result.push_back(hexChar(s.substr(0, firstGroupSize)));
      pos = firstGroupSize;
    }
    for (; pos < s.size(); pos += 4) {
      result.push_back(hexChar(s.substr(pos, 4)));
    }
    return result;
  }

} // namespace GlobalSim

#endif