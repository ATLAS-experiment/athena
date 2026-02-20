/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TGCSensitiveDetectorTool.h"
#include "TGCSensitiveDetector.h"

#include "HitManagement/HitCollectionMap.h"
#include "MuonSimEvent/TGCSimHitCollection.h"

TGCSensitiveDetectorTool::TGCSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

StatusCode TGCSensitiveDetectorTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<TGCSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode TGCSensitiveDetectorTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<TGCSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* TGCSensitiveDetectorTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  return new TGCSensitiveDetector(name(), m_outputCollectionNames[0]);
}
