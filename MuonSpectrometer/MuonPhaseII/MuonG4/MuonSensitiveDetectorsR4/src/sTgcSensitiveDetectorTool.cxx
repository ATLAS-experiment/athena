/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "sTgcSensitiveDetectorTool.h"
#include "sTgcSensitiveDetector.h"

namespace MuonG4R4 {

G4VSensitiveDetector* sTgcSensitiveDetectorTool::makeSD() const {
  return new sTgcSensitiveDetector(name(), m_outputCollectionNames[0], m_alignStoreKey, m_detMgr);
}
}
