/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HDF5Utils_HistAxis_H
#define HDF5Utils_HistAxis_H

#include <optional>
#include <string>
#include <vector>

namespace H5Utils::hist::detail {

  struct Axis
  {
    std::vector<float> edges{};
    std::string name{};
    bool underflow{false};
    bool overflow{false};
    bool is_int_or_category{false};
    // set iff the axis has equal-width bins (regular axis)
    std::optional<int> n_regular_bins{};
    bool operator==(const Axis&) const = default;
  };

 

} //> end namespace H5Utils::hist::detail

#endif //> !HDF5Utils_HistAxis_H
