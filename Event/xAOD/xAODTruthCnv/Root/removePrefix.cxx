// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "removePrefix.h"

namespace xAODMaker::Details {

std::string removePrefix(std::string_view str, std::string_view prefix) {

  if (str.starts_with(prefix)) {
    return std::string(str.substr(prefix.size()));
  }
  return std::string(str);
}

}  // namespace xAODMaker::Details
