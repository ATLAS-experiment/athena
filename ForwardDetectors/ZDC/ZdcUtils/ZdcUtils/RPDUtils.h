/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCUTILS_RPDUTILS_H
#define ZDCUTILS_RPDUTILS_H

#include <initializer_list>
#include <ranges>

namespace RPDUtils {
  unsigned int constexpr sideC = 0;
  unsigned int constexpr sideA = 1;
  std::initializer_list<unsigned int> constexpr sides {sideC, sideA};
  unsigned int ZDCSideToSideIndex(int);
  int constexpr ZDCSumsGlobalZDCSide = 0;
  unsigned int constexpr ZDCModuleZDCType = 0;
  unsigned int constexpr ZDCModuleRPDType = 1;
  unsigned int constexpr ZDCModuleEMModule = 0;
  unsigned int constexpr nChannels = 16;
  unsigned int constexpr nRows = 4;
  unsigned int constexpr nCols = 4;
  template <std::ranges::contiguous_range Range, typename T = std::ranges::range_value_t<Range>>
  void helpZero(Range & v) requires (std::is_arithmetic_v<T>) {
    std::fill(v.begin(), v.end(), 0);
  };
  template <std::ranges::contiguous_range OuterRange, typename InnerRange = std::ranges::range_value_t<OuterRange>, typename T = std::ranges::range_value_t<InnerRange>>
  void helpZero(OuterRange & vv) requires (std::ranges::contiguous_range<InnerRange> && std::is_arithmetic_v<T>) {
    for (auto & v : vv) {
      helpZero(v);
    }
  };
} // namespace RPDUtils

#endif
