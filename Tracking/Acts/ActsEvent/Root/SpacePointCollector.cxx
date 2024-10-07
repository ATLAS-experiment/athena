/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsEvent/SpacePointCollector.h"

namespace ActsTrk {
  
  SpacePointCollector::SpacePointCollector(std::vector<const xAOD::SpacePoint*>& externalStorage)
    : m_storage(&externalStorage)
    {}
  
} // namespace ActsTrk

