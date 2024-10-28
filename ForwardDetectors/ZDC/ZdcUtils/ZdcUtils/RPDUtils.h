/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCUTILS_RPDUTILS_H
#define ZDCUTILS_RPDUTILS_H

#include <initializer_list>
#include <ranges>
#include <iostream>
#include <sstream>
#include <vector>

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
  // implement printing of vector, required for gaudi property of type OptionalToolProperty<std::vector<T>>
  template<typename T>
  std::ostream & operator <<(std::ostream & os, const std::vector<T> & v);
  // the configuration string for RPD and centroid tools handles bulk setting of default reconstruction parameters
  // then, we would like the option to override each parameter in python configuration
  // therefore, we provide an std::optional-like implementation which
  // has the necessary interface to be a gaudi property
  template <class T>
  class OptionalToolProperty {
   public:
    OptionalToolProperty();
    explicit OptionalToolProperty(T value);
    bool has_value() const;
    T const& value() const;
    // enable printing; required for a gaudi property
    template <class V> friend std::ostream & operator<<(std::ostream &os, OptionalToolProperty<V> const& obj);
    // conversion to inner type for compatibility with gaudi parsers (reference to allow filling value);
    // this enables Python configuration (ZdcRecConfig.py) to set the corresponding tool property
    operator T&();
  private:
    T m_value {};
    bool m_hasValue = false;
  };
  // for debug printouts in RPD reconstruction
  template <typename T>
  std::string vecToString(std::vector<T> const& v);
} // namespace RPDUtils

#endif
