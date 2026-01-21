/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/****************************************************************
   Pixel Sensitive Detector class
   Davide Costanzo Sep 4th 2002.
 ****************************************************************/

#ifndef PIXEL_G4_SD_PIXELSENSORSD_H
#define PIXEL_G4_SD_PIXELSENSORSD_H

// Base class
#include "G4VSensitiveDetector.hh"

// use of the hits
#include "InDetSimEvent/SiHitCollection.h"
#include <gtest/gtest_prod.h>

class AtlasG4EventUserInfo;
// G4 needed classes
class G4Step;
class G4HCofThisEvent;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....

class PixelSensorSD : public G4VSensitiveDetector
{
 FRIEND_TEST( PixelSensorSDtest, ProcessHits );
 FRIEND_TEST( PixelSensorSDtest, AddHit );
 public:
  // Constructor
  PixelSensorSD(const std::string& name, const std::string& hitCollectionName);

  // Process the hits from G4
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

  // Initialize from G4
  void Initialize(G4HCofThisEvent *) override final;

  /** Templated method to stuff a single hit into the sensitive detector class.  This
   could get rather tricky, but the idea is to allow fast simulations to use the very
   same SD classes as the standard simulation. */
  template <class... Args> void AddHit(Args&&... args){ m_HitColl->Emplace( args... ); }

 private:
  // The hits collection
  std::string m_HitCollName;
  SiHitCollection* m_HitColl{nullptr};
  AtlasG4EventUserInfo* m_g4UserEventInfo{nullptr};
};

#endif //PIXEL_G4_SD_PIXELSENSORSD_H
