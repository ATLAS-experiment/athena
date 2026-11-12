/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FTagEigenVariations/SystematicNames.h"

#include <algorithm>
#include <format>

namespace {
  constexpr std::string_view prefix = "FT_EFF_";
}

std::string ftag::ev::eigenName(std::string_view label, Eigen::Index k) {
  return std::format("{}Eigen_{}_{}", prefix, label, k);
}

std::string ftag::ev::sourceName(std::string_view source) {
  std::string name(source);
  std::replace(name.begin(), name.end(), ' ', '_');
  if (!name.starts_with(prefix)) name.insert(0, prefix);
  return name;
}
