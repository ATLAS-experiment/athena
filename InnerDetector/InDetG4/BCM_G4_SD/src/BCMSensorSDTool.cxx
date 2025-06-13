/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//###############################################
//   BCM Sensitive Detector Tool
//
//###############################################

// Class header
#include "BCMSensorSDTool.h"

// For the SD itself
#include "BCMSensorSD.h"
#include "HitManagement/HitCollectionMap.h"
#include "InDetSimEvent/SiHitCollection.h"

BCMSensorSDTool::BCMSensorSDTool(const std::string& type, const std::string& name, const IInterface *parent) :
  SensitiveDetectorBase(type,name,parent)
{
}

StatusCode BCMSensorSDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<SiHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode BCMSensorSDTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<SiHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* BCMSensorSDTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );

  // Create a fresh SD
  return new BCMSensorSD(name(), m_outputCollectionNames[0]);
}


