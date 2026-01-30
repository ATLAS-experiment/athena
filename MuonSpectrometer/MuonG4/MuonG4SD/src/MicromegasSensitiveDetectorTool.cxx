/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MicromegasSensitiveDetectorTool.h"
#include "MicromegasSensitiveDetector.h"

#include "HitManagement/HitCollectionMap.h"
#include "MuonSimEvent/MMSimHitCollection.h"

MicromegasSensitiveDetectorTool::MicromegasSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

StatusCode MicromegasSensitiveDetectorTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<MMSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode MicromegasSensitiveDetectorTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<MMSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* MicromegasSensitiveDetectorTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  return new MicromegasSensitiveDetector(name(), m_outputCollectionNames[0]);
}
