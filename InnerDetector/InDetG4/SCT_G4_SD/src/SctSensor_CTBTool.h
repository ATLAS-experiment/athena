/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/****************************************************************
   SCT Sensitive Detector class
   Davide Costanzo Jul 14th 2003.
 ****************************************************************/

#ifndef SCT_G4_SD_SCTSENSOR_CTBTOOL_H
#define SCT_G4_SD_SCTSENSOR_CTBTOOL_H

// Base class
#include "G4AtlasTools/SensitiveDetectorBase.h"

// STL headers
#include <string>

class G4VSensitiveDetector;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....

class SctSensor_CTBTool : public SensitiveDetectorBase
{
 public:
  // Constructor
  SctSensor_CTBTool(const std::string& type, const std::string& name, const IInterface *parent);

  virtual StatusCode SetupEvent(HitCollectionMap&) override;
  virtual StatusCode Gather(HitCollectionMap&) override;

protected:
  // Make me an SD!
  G4VSensitiveDetector* makeSD() const override final;

};

#endif //SCT_G4_SD_SCTSENSOR_CTBTOOL_H
