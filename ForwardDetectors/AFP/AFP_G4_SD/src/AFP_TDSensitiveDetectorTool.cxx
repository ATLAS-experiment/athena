/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


// Class header
#include "AFP_TDSensitiveDetectorTool.h"

// For the SD itself
#include "AFP_HitCollectionBuilders.h"
#include "AFP_TDSensitiveDetector.h"
#include "HitManagement/HitCollectionMap.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

AFP_TDSensitiveDetectorTool::AFP_TDSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase(type,name,parent)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode AFP_TDSensitiveDetectorTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<AFP_TDSimHitCollectionBuilder>(m_outputCollectionNames[0],
                                                        m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode AFP_TDSensitiveDetectorTool::Gather(HitCollectionMap& hitCollections)
{
  ATH_MSG_VERBOSE( "AFP_TDSensitiveDetectorTool::Gather()" );
  CHECK(hitCollections.Record<AFP_TDSimHitCollection>(m_outputCollectionNames[0]));
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* AFP_TDSensitiveDetectorTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  // Create a fresh SD
  return new AFP_TDSensitiveDetector(name(), m_outputCollectionNames[0]);
}
