/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "CSCSensitiveDetectorCosmicsTool.h"
#include "CSCSensitiveDetectorCosmics.h"

#include "HitManagement/HitCollectionMap.h"
#include "MuonSimEvent/CSCSimHitCollection.h"

CSCSensitiveDetectorCosmicsTool::CSCSensitiveDetectorCosmicsTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

StatusCode CSCSensitiveDetectorCosmicsTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<CSCSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode CSCSensitiveDetectorCosmicsTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<CSCSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* CSCSensitiveDetectorCosmicsTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  return new CSCSensitiveDetectorCosmics(name(), m_outputCollectionNames[0]);
}
