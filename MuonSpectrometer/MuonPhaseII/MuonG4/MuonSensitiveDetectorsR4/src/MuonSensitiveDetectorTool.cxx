/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSensitiveDetectorTool.h"
#include "MuonSensitiveDetector.h"

namespace MuonG4R4{

    StatusCode MuonSensitiveDetectorTool::initialize() {
        ATH_CHECK(SensitiveDetectorBase::initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }
    StatusCode MuonSensitiveDetectorTool::SetupEvent(HitCollectionMap& hitCollections) {
        hitCollections.Emplace<MuonSimHitsVec>(m_outputCollectionNames[0]);
 
        return StatusCode::SUCCESS;
    }
    StatusCode MuonSensitiveDetectorTool::Gather(HitCollectionMap& hitCollections) {
        hitCollections.Record<MuonSimHitsVec>(m_outputCollectionNames[0]);
        return StatusCode::SUCCESS;
    }
}