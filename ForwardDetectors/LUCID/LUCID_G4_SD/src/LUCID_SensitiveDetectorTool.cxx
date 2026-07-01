/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


// Class header
#include "LUCID_SensitiveDetectorTool.h"

// Package includes
#include "LUCID_SensitiveDetector.h"

#include "HitManagement/HitCollectionMap.h"
#include "LUCID_SimEvent/LUCID_SimHitCollection.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

LUCID_SensitiveDetectorTool::LUCID_SensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase(type,name,parent)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode LUCID_SensitiveDetectorTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<LUCID_SimHitCollection>(m_outputCollectionNames[0],
                                                 m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode LUCID_SensitiveDetectorTool::Gather(HitCollectionMap& hitCollections)
{
  CHECK(hitCollections.Record<LUCID_SimHitCollection>(m_outputCollectionNames[0]));
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* LUCID_SensitiveDetectorTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  // Create a fresh SD
  return new LUCID_SensitiveDetector(name(), m_outputCollectionNames[0]);
}
