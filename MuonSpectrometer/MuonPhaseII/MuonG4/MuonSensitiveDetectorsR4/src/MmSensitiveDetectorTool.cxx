/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MmSensitiveDetectorTool.h"
#include "MmSensitiveDetector.h"

namespace MuonG4R4 {

G4VSensitiveDetector* MmSensitiveDetectorTool::makeSD() const {
  return new MmSensitiveDetector(name(), m_outputCollectionNames[0], m_alignStoreKey, m_detMgr);
}
}
