/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/****************************************************************
   Track Recorder Sensitive Detector Tool
 ****************************************************************/

#ifndef TRACKWRITEFASTSIM_TRACKRECORDERSDTOOL_H
#define TRACKWRITEFASTSIM_TRACKRECORDERSDTOOL_H

// Base class
#include "G4AtlasTools/SensitiveDetectorBase.h"

class G4VSensitiveDetector;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....

class TrackRecorderSDTool : public SensitiveDetectorBase
{
 public:
  using SensitiveDetectorBase::SensitiveDetectorBase;

protected:
  StatusCode SetupEvent(HitCollectionMap&) override final;
  StatusCode Gather(HitCollectionMap&) override final;

  // Make me an SD!
  G4VSensitiveDetector* makeSD() const override final;

  Gaudi::Property<int> m_SD_type{this, "SD_type", 1, "TrackRecorderSD=1, CosmicTRSD=2"};
};

#endif //TRACKWRITEFASTSIM_TRACKRECORDERSDTOOL_H
