/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


// Class header
#include "AFP_SiDSensitiveDetectorTool.h"

// For the SD itself
#include "AFP_HitCollectionBuilders.h"
#include "AFP_SiDSensitiveDetector.h"
#include "HitManagement/HitCollectionMap.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

AFP_SiDSensitiveDetectorTool::AFP_SiDSensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase(type,name,parent)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode AFP_SiDSensitiveDetectorTool::SetupEvent(HitCollectionMap& hitCollections)
{
  ATH_MSG_VERBOSE( "AFP_SiDSensitiveDetectorTool::SetupEvent()" );
  hitCollections.Emplace<AFP_SIDSimHitCollectionBuilder>(m_outputCollectionNames[0],
                                                         m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode AFP_SiDSensitiveDetectorTool::Gather(HitCollectionMap& hitCollections)
{
  ATH_MSG_VERBOSE( "AFP_SiDSensitiveDetectorTool::Gather()" );
  CHECK(hitCollections.Record<AFP_SIDSimHitCollection>(m_outputCollectionNames[0]));
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* AFP_SiDSensitiveDetectorTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  // Create a fresh SD
  return new AFP_SiDSensitiveDetector(name(), m_outputCollectionNames[0]);
}
