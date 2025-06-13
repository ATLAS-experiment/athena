/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/****************************************************************
   SCT Sensitive Detector Tool
 ****************************************************************/

#ifndef SCT_G4_SD_SCTSENSORSDTOOL_H
#define SCT_G4_SD_SCTSENSORSDTOOL_H

// Base class
#include "G4AtlasTools/SensitiveDetectorBase.h"

#include "GeoModelInterfaces/IGeoDbTagSvc.h"

// STL headers
#include <string>

class G4VSensitiveDetector;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....

class SctSensorSDTool : public SensitiveDetectorBase
{
 public:
  // Constructor
  SctSensorSDTool(const std::string& type, const std::string& name, const IInterface *parent);

  virtual StatusCode SetupEvent(HitCollectionMap&) override;
  virtual StatusCode Gather(HitCollectionMap&) override;

protected:
  // Make me an SD!
  G4VSensitiveDetector* makeSD() const override final;
  bool m_isGmxSensor;
  ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this, "GeoDbTagSvc", "GeoDbTagSvc", ""};
  

};

#endif //SCT_G4_SD_SCTSENSORSDTOOL_H
