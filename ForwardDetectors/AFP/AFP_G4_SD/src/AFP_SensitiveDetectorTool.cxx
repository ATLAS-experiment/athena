/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


// Class header
#include "AFP_SensitiveDetectorTool.h"

// For the SD itself
#include "AFP_HitCollectionBuilders.h"
#include "AFP_SensitiveDetector.h"
#include "HitManagement/HitCollectionMap.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

AFP_SensitiveDetectorTool::AFP_SensitiveDetectorTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase(type,name,parent)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode AFP_SensitiveDetectorTool::SetupEvent(HitCollectionMap& hitCollections)
{
  ATH_MSG_VERBOSE( "AFP_SensitiveDetectorTool::SetupEvent()" );
  hitCollections.Emplace<AFP_TDSimHitCollectionBuilder>(m_outputCollectionNames[0],
                                                        m_outputCollectionNames[0]);
  hitCollections.Emplace<AFP_SIDSimHitCollectionBuilder>(m_outputCollectionNames[1],
                                                         m_outputCollectionNames[1]);
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode AFP_SensitiveDetectorTool::Gather(HitCollectionMap& hitCollections)
{
  ATH_MSG_VERBOSE( "AFP_SensitiveDetectorTool::Gather()" );
  CHECK(hitCollections.Record<AFP_TDSimHitCollection>(m_outputCollectionNames[0]));
  CHECK(hitCollections.Record<AFP_SIDSimHitCollection>(m_outputCollectionNames[1]));
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* AFP_SensitiveDetectorTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  // Create a fresh SD
  return new AFP_SensitiveDetector(name(), m_outputCollectionNames[0], m_outputCollectionNames[1]);
}
