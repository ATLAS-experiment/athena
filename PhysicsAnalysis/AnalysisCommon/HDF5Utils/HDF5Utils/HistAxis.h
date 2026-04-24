/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HDF5Utils_HistAxis_H
#define HDF5Utils_HistAxis_H

#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace H5Utils::hist::detail {

struct regular_axis_t {
  double lower{};
  double upper{};
  size_t n_bins{};
  bool operator==(const regular_axis_t&) const = default;
};

struct Axis
{
  // Encodes the axis kind:
  //   regular_axis_t       – equal-width float axis (lower, upper, n_bins)
  //   vector<double>       – variable-width float axis (N+1 edges for N bins)
  //   vector<int64_t>      – non-contiguous integer category values
  //   vector<string>       – string category labels
  //   pair<int64_t,int64_t>– contiguous integer range [first, last] inclusive
  using edges_t = std::variant<
    regular_axis_t,
    std::vector<double>,
    std::vector<int64_t>,
    std::vector<std::string>,
    std::pair<int64_t,int64_t>
  >;
  edges_t edges{regular_axis_t{}};
  std::string name{};
  bool underflow{false};
  bool overflow{false};
  bool operator==(const Axis&) const = default;
};

} // namespace H5Utils::hist::detail

#endif //> !HDF5Utils_HistAxis_H
