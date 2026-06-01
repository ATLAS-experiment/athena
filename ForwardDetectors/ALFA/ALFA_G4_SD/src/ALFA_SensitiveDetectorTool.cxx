/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


// Class header
#include "ALFA_SensitiveDetectorTool.h"

// Package includes
#include "ALFA_SensitiveDetector.h"

#include "ALFA_SimEv/ALFA_HitCollection.h"
#include "ALFA_SimEv/ALFA_ODHitCollection.h"
#include "HitManagement/HitCollectionMap.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ALFA_SensitiveDetectorTool::ALFA_SensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase(type,name,parent)
{
  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode ALFA_SensitiveDetectorTool::SetupEvent(HitCollectionMap& hitCollections)
{
  ATH_MSG_VERBOSE( "ALFA_SensitiveDetectorTool::SetupEvent()" );
  hitCollections.Emplace<ALFA_HitCollection>(m_outputCollectionNames[0],
                                             m_outputCollectionNames[0]);
  hitCollections.Emplace<ALFA_ODHitCollection>(m_outputCollectionNames[1],
                                               m_outputCollectionNames[1]);
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode ALFA_SensitiveDetectorTool::Gather(HitCollectionMap& hitCollections)
{
  ATH_MSG_VERBOSE( "ALFA_SensitiveDetectorTool::Gather()" );
  CHECK(hitCollections.Record<ALFA_HitCollection>(m_outputCollectionNames[0]));
  CHECK(hitCollections.Record<ALFA_ODHitCollection>(m_outputCollectionNames[1]));
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* ALFA_SensitiveDetectorTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  // Create a fresh SD
  return new ALFA_SensitiveDetector(name(), m_outputCollectionNames[0], m_outputCollectionNames[1]);
}
