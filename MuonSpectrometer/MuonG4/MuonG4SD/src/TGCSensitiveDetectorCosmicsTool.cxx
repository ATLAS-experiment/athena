/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TGCSensitiveDetectorCosmicsTool.h"
#include "TGCSensitiveDetectorCosmics.h"

#include "HitManagement/HitCollectionMap.h"
#include "MuonSimEvent/TGCSimHitCollection.h"

TGCSensitiveDetectorCosmicsTool::TGCSensitiveDetectorCosmicsTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

StatusCode TGCSensitiveDetectorCosmicsTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<TGCSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode TGCSensitiveDetectorCosmicsTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<TGCSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* TGCSensitiveDetectorCosmicsTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  return new TGCSensitiveDetectorCosmics(name(), m_outputCollectionNames[0]);
}
