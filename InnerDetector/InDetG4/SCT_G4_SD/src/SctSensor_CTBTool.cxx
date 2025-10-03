/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// SCT Sensitive Detector Tool.
//

//class header
#include "SctSensor_CTBTool.h"

//package includes
#include "SctSensor_CTB.h"
#include "HitManagement/HitCollectionMap.h"
#include "InDetSimEvent/SiHitCollection.h"

// STL includes
#include <exception>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SctSensor_CTBTool::SctSensor_CTBTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

StatusCode SctSensor_CTBTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<SiHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode SctSensor_CTBTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<SiHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* SctSensor_CTBTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );

  return new SctSensor_CTB(name(), m_outputCollectionNames[0]);
}

