/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// SCT Sensitive Detector Tool.
//

// class header
#include "SctSensorSDTool.h"

// package includes
#include "SctSensorSD.h"
#include "SctSensorGmxSD.h"

#include <GeoModelRead/ReadGeoModel.h>
#include "HitManagement/HitCollectionMap.h"
#include "InDetSimEvent/SiHitCollection.h"

// STL includes
#include <exception>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SctSensorSDTool::SctSensorSDTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent ),m_isGmxSensor(false)
{
  declareProperty("GmxSensor",m_isGmxSensor);
}

StatusCode SctSensorSDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<SiHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode SctSensorSDTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<SiHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* SctSensorSDTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  GeoModelIO::ReadGeoModel* sqlreader = nullptr;
  StatusCode sc = m_geoDbTagSvc.retrieve();
  if (sc.isFailure()) {
    msg(MSG::ERROR) << "Could not locate GeoDbTagSvc" << endmsg;
    }
  else {
      sqlreader = m_geoDbTagSvc->getSqliteReader();
  }

  if(m_isGmxSensor)
    {
      return new SctSensorGmxSD(name(), m_outputCollectionNames[0],sqlreader);
    }
  else
    {
      return new SctSensorSD(name(), m_outputCollectionNames[0]);
    }
}

