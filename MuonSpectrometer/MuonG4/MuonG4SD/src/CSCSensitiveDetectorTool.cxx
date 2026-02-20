/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "CSCSensitiveDetectorTool.h"
#include "CSCSensitiveDetector.h"

#include "HitManagement/HitCollectionMap.h"
#include "MuonSimEvent/CSCSimHitCollection.h"

CSCSensitiveDetectorTool::CSCSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

StatusCode CSCSensitiveDetectorTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<CSCSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode CSCSensitiveDetectorTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<CSCSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* CSCSensitiveDetectorTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  return new CSCSensitiveDetector(name(), m_outputCollectionNames[0]);
}
