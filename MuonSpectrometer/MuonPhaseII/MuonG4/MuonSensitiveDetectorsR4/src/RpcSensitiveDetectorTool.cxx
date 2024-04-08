/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "RpcSensitiveDetectorTool.h"
#include "RpcSensitiveDetector.h"

namespace MuonG4R4 {

RpcSensitiveDetectorTool::RpcSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent ) {}

StatusCode RpcSensitiveDetectorTool::initialize() {
    ATH_CHECK(SensitiveDetectorBase::initialize());
    ATH_CHECK(detStore()->retrieve(m_detMgr));
    return StatusCode::SUCCESS;
}
G4VSensitiveDetector* RpcSensitiveDetectorTool::makeSD() const {
  return new RpcSensitiveDetector(name(), m_outputCollectionNames[0], m_alignStoreKey, m_detMgr);
}
}
