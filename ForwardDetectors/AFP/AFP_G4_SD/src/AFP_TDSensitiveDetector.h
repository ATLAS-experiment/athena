/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFP_G4_SD_AFP_TDSensitiveDetector_h
#define AFP_G4_SD_AFP_TDSensitiveDetector_h

// Base class
#include "G4VSensitiveDetector.hh"

// use of the hits
#include "AFP_HitCollectionBuilders.h"
#include <gtest/gtest_prod.h>

// STL header
#include <string>
#include <utility>

class G4Step;
class G4TouchableHistory;

class AFP_TDSensitiveDetector : public G4VSensitiveDetector
{
 FRIEND_TEST( AFP_TDSensitiveDetectortest, ProcessHits );
 FRIEND_TEST( AFP_TDSensitiveDetectortest, AddHit );

public:
  // Constructor
  AFP_TDSensitiveDetector(const std::string& name, const std::string& hitCollectionName);

  // Destructor
  ~AFP_TDSensitiveDetector() { /* I don't own myHitColl if all has gone well */ }

  // Called from G4 at the start of each G4 event
  void Initialize(G4HCofThisEvent *) override final;
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

  /** Templated method to stuff a single hit into the sensitive detector class.  This
   could get rather tricky, but the idea is to allow fast simulations to use the very
   same SD classes as the standard simulation. */
  template <class... Args> void AddHit(Args&&... args)
  {
    if (m_HitColl) {
      m_HitColl->Emplace(std::forward<Args>(args)...);
    }
  }
  
  static constexpr double TDMaxQEff = 0.15;

private:
  AFP_TDSimHitCollectionBuilder* getHitCollection() const;

  int m_nHitID;
  // The hits collection
  std::string m_hitCollectionName;
  // Non-owning cache set by Initialize; HitCollectionMap owns the collection.
  AFP_TDSimHitCollectionBuilder* m_HitColl{};
};

#endif //AFP_G4_SD_AFP_TDSensitiveDetector_h
