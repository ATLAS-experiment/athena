/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//###############################################
//   BCM Sensitive Detector class
//   Bostjan Macek 17.may.2007
//###############################################

#ifndef BCMSENSORSD_H
#define BCMSENSORSD_H

// Base class
#include "G4VSensitiveDetector.hh"

class AtlasG4EventUserInfo;
// G4 needed classes
class G4Step;

// use of the hits
#include "InDetSimEvent/SiHitCollection.h"
#include <gtest/gtest_prod.h>

class BCMSensorSD : public G4VSensitiveDetector
{
 FRIEND_TEST( BCMSensorSDtest, ProcessHits );
 FRIEND_TEST( BCMSensorSDtest, AddHit );
 public:
  // Constructor
  BCMSensorSD(const std::string& name, const std::string& hitCollectionName);

  // Process the hits from G4
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

  // Initialize from G4
  void Initialize(G4HCofThisEvent *) override final;

  /** Templated method to stuff a single hit into the sensitive detector class.  This
   could get rather tricky, but the idea is to allow fast simulations to use the very
   same SD classes as the standard simulation. */
  template <class... Args> void AddHit(Args&&... args){ m_HitColl->Emplace( args... ); }
 private:
  std::string m_HitCollName; //!< Name of the hit collection
  SiHitCollection* m_HitColl{nullptr}; //!< Pointer to the hit collection
  AtlasG4EventUserInfo* m_g4UserEventInfo{nullptr};
};

#endif
