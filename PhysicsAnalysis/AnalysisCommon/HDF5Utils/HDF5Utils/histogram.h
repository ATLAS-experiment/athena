/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef H5_HISTOGRAMS_H
#define H5_HISTOGRAMS_H

namespace H5 {
  class Group;
}

#include <string>

namespace H5Utils::hist {
  template <typename T>
  void write_hist_to_group(
    H5::Group& group,
    const T& hist,
    const std::string& name);
}

#include "histogram.icc"

#endif
