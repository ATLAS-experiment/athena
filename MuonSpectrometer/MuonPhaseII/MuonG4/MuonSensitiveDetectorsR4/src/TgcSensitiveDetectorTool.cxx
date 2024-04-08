/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "TgcSensitiveDetectorTool.h"
#include "TgcSensitiveDetector.h"

namespace MuonG4R4 {

TgcSensitiveDetectorTool::TgcSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent ) {}

StatusCode TgcSensitiveDetectorTool::initialize() {
    ATH_CHECK(SensitiveDetectorBase::initialize());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
G4VSensitiveDetector* TgcSensitiveDetectorTool::makeSD() const {
  return new TgcSensitiveDetector(name(), m_outputCollectionNames[0], m_alignStoreKey, m_detMgr);
}
}
