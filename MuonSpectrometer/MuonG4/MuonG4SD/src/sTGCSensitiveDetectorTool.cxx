/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "sTGCSensitiveDetectorTool.h"
#include "sTGCSensitiveDetector.h"

#include "HitManagement/HitCollectionMap.h"
#include "MuonSimEvent/sTGCSimHitCollection.h"

sTGCSensitiveDetectorTool::sTGCSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

G4VSensitiveDetector* sTGCSensitiveDetectorTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  return new sTGCSensitiveDetector(name(), m_outputCollectionNames[0], m_onSqLite);
}

StatusCode sTGCSensitiveDetectorTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<sTGCSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode sTGCSensitiveDetectorTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<sTGCSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}
