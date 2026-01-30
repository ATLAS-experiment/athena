/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/****************************************************************
   SCT Sensitive Detector class
   Davide Costanzo Jul 14th 2003.
 ****************************************************************/

#ifndef SCT_G4_SD_SCTSENSOR_CTB_H
#define SCT_G4_SD_SCTSENSOR_CTB_H

// Base class
#include "G4VSensitiveDetector.hh"

// For the hits
#include "InDetSimEvent/SiHitCollection.h"
#include <gtest/gtest_prod.h>

class AtlasG4EventUserInfo;
// G4 needed classes
class G4Step;
class G4TouchableHistory;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....

class SctSensor_CTB : public G4VSensitiveDetector
{
 FRIEND_TEST( SctSensor_CTBtest, ProcessHits );
 FRIEND_TEST( SctSensor_CTBtest, AddHit );
 public:
  // Constructor
  SctSensor_CTB( const std::string& name, const std::string& hitCollectionName );

  // Process each G4 hit
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

  // For setting up the hit collection
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

#endif //SCT_G4_SD_SCTSENSOR_CTB_H
