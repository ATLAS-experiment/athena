/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtSensitiveDetectorTool.h"
#include "MdtSensitiveDetector.h"

namespace MuonG4R4 {

G4VSensitiveDetector* MdtSensitiveDetectorTool::makeSD() const {
  return new MdtSensitiveDetector(name(), m_outputCollectionNames[0], m_alignStoreKey, m_detMgr);
}
}
