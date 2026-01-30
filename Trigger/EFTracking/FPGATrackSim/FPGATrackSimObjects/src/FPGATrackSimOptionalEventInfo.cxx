/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimObjects/FPGATrackSimOptionalEventInfo.h"
#include <iostream>

FPGATrackSimOptionalEventInfo::~FPGATrackSimOptionalEventInfo() {
  reset();
}

void FPGATrackSimOptionalEventInfo::reset() const {
  m_OfflineClusters.clear();
  m_OfflineTracks.clear();
  m_TruthTracks.clear();
}

std::ostream& operator<<(std::ostream& s, const FPGATrackSimOptionalEventInfo& info) {
  s << "nOfflineClusters: " << info.nOfflineClusters() << ", "
    << "nOfflineTracks: " << info.nOfflineTracks() << ", "
    << "nTruthTracks: " << info.nTruthTracks() << std::endl;
  return s;
}
