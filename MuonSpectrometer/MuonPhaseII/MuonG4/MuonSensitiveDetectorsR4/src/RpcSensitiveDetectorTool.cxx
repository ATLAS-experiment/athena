/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "RpcSensitiveDetectorTool.h"
#include "RpcSensitiveDetector.h"

namespace MuonG4R4 {

G4VSensitiveDetector* RpcSensitiveDetectorTool::makeSD() const {
  return new RpcSensitiveDetector(name(), m_outputCollectionNames[0], m_alignStoreKey, m_detMgr);
}
}
