/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "sTgcSensitiveDetectorTool.h"
#include "sTgcSensitiveDetector.h"

namespace MuonG4R4 {

sTgcSensitiveDetectorTool::sTgcSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent ) {}

StatusCode sTgcSensitiveDetectorTool::initialize() {
    ATH_CHECK(SensitiveDetectorBase::initialize());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
G4VSensitiveDetector* sTgcSensitiveDetectorTool::makeSD() const {
  return new sTgcSensitiveDetector(name(), m_outputCollectionNames[0], m_alignStoreKey, m_detMgr);
}
}
