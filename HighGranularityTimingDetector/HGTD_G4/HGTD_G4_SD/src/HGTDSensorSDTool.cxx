/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// HGTD Sensitive Detector.
// The Hits are processed here. For every hit, the position and information
// on the sensor in which the interaction happened are obtained.

// Class header
#include "HGTDSensorSDTool.h"

// For the SD itself
#include "HGTDSensorSD.h"
#include "HGTDSensorGmxSD.h"

#include <GeoModelRead/ReadGeoModel.h>
#include "HitManagement/HitCollectionMap.h"
#include "InDetSimEvent/SiHitCollection.h"

// STL includes
#include <exception>

HGTDSensorSDTool::HGTDSensorSDTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

StatusCode HGTDSensorSDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<SiHitCollection>(m_outputCollectionNames[0],
                                          m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode HGTDSensorSDTool::Gather(HitCollectionMap& hitCollections)
{
  CHECK(hitCollections.Record<SiHitCollection>(m_outputCollectionNames[0]));
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* HGTDSensorSDTool::makeSD() const
{
  // Make sure the job has been set up properly
  ATH_MSG_DEBUG( "Initializing SD" );
  GeoModelIO::ReadGeoModel* sqlreader = nullptr;
  StatusCode sc = m_geoDbTagSvc.retrieve();
  if (sc.isFailure()) {
    msg(MSG::ERROR) << "Could not locate GeoDbTagSvc" << endmsg;
    }
  else {
    sqlreader = m_geoDbTagSvc->getSqliteReader();
  }
  // Create a fresh SD
  if (!m_gmxSensor){
    return new HGTDSensorSD(name(), m_outputCollectionNames[0]);
  } else {
    return new HGTDSensorGmxSD(name(), m_outputCollectionNames[0], sqlreader);
  }
  
}
