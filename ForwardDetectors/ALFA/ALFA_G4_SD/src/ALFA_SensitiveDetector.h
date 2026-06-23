/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ALFA_G4_SD_ALFA_SensitiveDetector_h
#define ALFA_G4_SD_ALFA_SensitiveDetector_h

// Base class header file
#include "G4VSensitiveDetector.hh"

// use of the hits
#include "ALFA_SimEv/ALFA_HitCollection.h"
#include "ALFA_SimEv/ALFA_ODHitCollection.h"
#include <gtest/gtest_prod.h>

// STL header
#include <string>

class G4Step;
class G4TouchableHistory;

class ALFA_SensitiveDetector : public G4VSensitiveDetector
{
 FRIEND_TEST( ALFA_SensitiveDetectortest, ProcessHits1 );
 FRIEND_TEST( ALFA_SensitiveDetectortest, ProcessHits2 );
 FRIEND_TEST( ALFA_SensitiveDetectortest, ProcessHits3 );

public:
  // Constructor
  ALFA_SensitiveDetector(const std::string& name, const std::string& hitCollectionName, const std::string& ODhitCollectionName);
  // Destructor
  ~ALFA_SensitiveDetector() {}

  // Called from G4 at the start of each G4 event
  void Initialize(G4HCofThisEvent *) override final;
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

  /** Templated method to stuff a single hit into the sensitive detector class.  This
   could get rather tricky, but the idea is to allow fast simulations to use the very
   same SD classes as the standard simulation. */
  //template <class... Args> void AddHit(Args&&... args){ m_HitColl->Emplace( args... ); }

private:
  ALFA_HitCollection* getHitCollection() const;
  ALFA_ODHitCollection* getODHitCollection() const;

  int m_hitID{};

  // The hits collections
  std::string m_hitCollectionName;
  std::string m_ODHitCollectionName;
  // Non-owning caches set by Initialize; HitCollectionMap owns the collections.
  ALFA_HitCollection* m_HitCollection{};
  ALFA_ODHitCollection* m_ODHitCollection{};

  int m_pos1{}, m_pos2{};

  int m_num[3]{};
};

#endif //ALFA_G4_SD_ALFA_SensitiveDetector_h
