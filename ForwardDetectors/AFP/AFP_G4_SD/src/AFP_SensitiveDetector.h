/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFP_G4_SD_AFP_SensitiveDetector_h
#define AFP_G4_SD_AFP_SensitiveDetector_h

// Base class
#include "G4VSensitiveDetector.hh"

// use of the hits
#include "AFP_HitCollectionBuilders.h"
#include <gtest/gtest_prod.h>

// STL header
#include <string>

class G4Step;
class G4TouchableHistory;


class AFP_SensitiveDetector : public G4VSensitiveDetector
{
 FRIEND_TEST( AFP_SensitiveDetectortest, ProcessHits1 );
 FRIEND_TEST( AFP_SensitiveDetectortest, ProcessHits2 );

public:
  // Constructor
  AFP_SensitiveDetector(const std::string& name, const std::string& TDhitCollectionName, const std::string& SIDhitCollectionName);

  // Destructor
  ~AFP_SensitiveDetector() { /* I don't own myHitColl if all has gone well */ }

  // Called from G4 at the start of each G4 event
  void Initialize(G4HCofThisEvent *) override final;
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

  /** Templated method to stuff a single hit into the sensitive detector class.  This
   could get rather tricky, but the idea is to allow fast simulations to use the very
   same SD classes as the standard simulation. */
  //template <class... Args> void AddHit(Args&&... args){ m_HitColl->Emplace( args... ); }
  
  static constexpr double TDMaxQEff = 0.15;

private:
  AFP_TDSimHitCollectionBuilder* getTDHitCollection() const;
  AFP_SIDSimHitCollectionBuilder* getSIDHitCollection() const;

  int m_nHitID;

  float m_delta_pixel_x, m_delta_pixel_y;
  float m_death_edge[4][10];
  float m_lower_edge[4][10];

  // The hits collections
  std::string m_TDHitCollectionName;
  std::string m_SIDHitCollectionName;
  // Non-owning caches set by Initialize; HitCollectionMap owns the collections.
  AFP_TDSimHitCollectionBuilder* m_pTDSimHitCollection{};
  AFP_SIDSimHitCollectionBuilder* m_pSIDSimHitCollection{};

};

#endif //AFP_G4_SD_AFP_SensitiveDetector_h
