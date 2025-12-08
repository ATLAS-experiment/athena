/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GenericMuonSensitiveDetectorTool.h"
#include "GenericMuonSensitiveDetector.h"

#include "HitManagement/HitCollectionMap.h"
#include "MuonSimEvent/GenericMuonSimHitCollection.h"

GenericMuonSensitiveDetectorTool::GenericMuonSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

StatusCode GenericMuonSensitiveDetectorTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<GenericMuonSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode GenericMuonSensitiveDetectorTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<GenericMuonSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* GenericMuonSensitiveDetectorTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  return new GenericMuonSensitiveDetector(name(), m_outputCollectionNames[0]);
}
