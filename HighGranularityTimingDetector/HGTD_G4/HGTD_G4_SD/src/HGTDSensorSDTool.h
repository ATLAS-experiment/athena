/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////
// HGTDSensorSDTool.h, (c) ATLAS Detector software
//////////////////////////////////////////////////////////

#ifndef HGTD_G4_SD_HGTDSENSORSDTOOL_H
#define HGTD_G4_SD_HGTDSENSORSDTOOL_H

// Base class
#include "G4AtlasTools/SensitiveDetectorBase.h"

#include "GeoModelInterfaces/IGeoDbTagSvc.h"

// STL headers
#include <string>

class G4VSensitiveDetector;

class HGTDSensorSDTool : public SensitiveDetectorBase
{
public:
  // Constructor
  HGTDSensorSDTool(const std::string& type, const std::string& name, const IInterface *parent);
    
  // Destructor
  virtual ~HGTDSensorSDTool() {}

protected:
  // Create the event-owned hit collection.
  virtual StatusCode SetupEvent(HitCollectionMap&) override final;
  // Record the event-owned hit collection to StoreGate.
  virtual StatusCode Gather(HitCollectionMap&) override final;
  // Make me an SD!
  virtual G4VSensitiveDetector* makeSD() const override final;
  // property to toggle GeoModelXML 
  Gaudi::Property<bool> m_gmxSensor{this, "GmxSensor", false};
  ServiceHandle<IGeoDbTagSvc> m_geoDbTagSvc{this, "GeoDbTagSvc", "GeoDbTagSvc", ""};
};

#endif // HGTD_G4_SD_HGTDSENSORSDTOOL_H
