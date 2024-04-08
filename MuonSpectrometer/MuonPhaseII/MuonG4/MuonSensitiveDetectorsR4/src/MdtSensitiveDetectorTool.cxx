/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtSensitiveDetectorTool.h"
#include "MdtSensitiveDetector.h"

namespace MuonG4R4 {

MdtSensitiveDetectorTool::MdtSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent ) {}

StatusCode MdtSensitiveDetectorTool::initialize() {
    ATH_CHECK(SensitiveDetectorBase::initialize());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
G4VSensitiveDetector* MdtSensitiveDetectorTool::makeSD() const {
  return new MdtSensitiveDetector(name(), m_outputCollectionNames[0], m_alignStoreKey, m_detMgr);
}
}
