/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Pixel Sensitive Detector.
// The Hits are processed here. For every hit I get the position and
// an information on the sensor in which the interaction happened

// Class header
#include "PixelSensorSDTool.h"

// For the SD itself
#include "PixelSensorSD.h"
#include "PixelSensorGmxSD.h"

#include "HitManagement/HitCollectionMap.h"
#include "InDetSimEvent/SiHitCollection.h"
#include <GeoModelRead/ReadGeoModel.h>

// STL includes
#include <exception>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PixelSensorSDTool::PixelSensorSDTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

StatusCode PixelSensorSDTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<SiHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode PixelSensorSDTool::Gather(HitCollectionMap& hitCollections)
{
  return hitCollections.Record<SiHitCollection>(m_outputCollectionNames[0]);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* PixelSensorSDTool::makeSD() const
{
  // Make sure the job has been set up properly
  ATH_MSG_DEBUG( "Initializing SD" );
  const bool diagnoseBCMPrime = !m_outputCollectionNames.empty() &&
                                (m_outputCollectionNames[0] == "BCMPrimeHits" ||
                                 m_outputCollectionNames[0] == "BCMPrimeHits_G4");
  if (diagnoseBCMPrime) {
    ATH_MSG_DEBUG("BCMPrimeSensorSD diagnostic: PixelSensorSDTool::makeSD for tool '" << name()
                 << "', GmxSensor=" << (m_gmxSensor.value() ? "true" : "false")
                 << ", outputCollection=" << m_outputCollectionNames[0]
                 << ", LogicalVolumeNames=" << m_volumeNames.value());
  }
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
    if (diagnoseBCMPrime) {
      ATH_MSG_DEBUG("BCMPrimeSensorSD diagnostic: creating PixelSensorSD");
    }
    return new PixelSensorSD(name(), m_outputCollectionNames[0]);
  } else {
    if (diagnoseBCMPrime) {
      ATH_MSG_DEBUG("BCMPrimeSensorSD diagnostic: creating PixelSensorGmxSD, sqlreader="
                   << (sqlreader ? "set" : "null"));
    }
    return new PixelSensorGmxSD(name(), m_outputCollectionNames[0],sqlreader);
  }
}

