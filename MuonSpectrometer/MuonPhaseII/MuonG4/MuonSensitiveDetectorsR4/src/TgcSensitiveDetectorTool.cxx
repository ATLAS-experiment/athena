/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TgcSensitiveDetectorTool.h"
#include "TgcSensitiveDetector.h"

namespace MuonG4R4 {
G4VSensitiveDetector* TgcSensitiveDetectorTool::makeSD() const {
  return new TgcSensitiveDetector(name(), m_outputCollectionNames[0], m_alignStoreKey, m_detMgr);
}
}
