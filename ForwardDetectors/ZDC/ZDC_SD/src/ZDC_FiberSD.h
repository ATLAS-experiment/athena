/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_SD_ZDC_FIBER_SD_H
#define ZDC_SD_ZDC_FIBER_SD_H

// Base class
#include "G4VSensitiveDetector.hh"

// use of the hits
#include "ZDC_HitCollectionBuilders.h"

// STL header
#include <string>
#include <gtest/gtest_prod.h>

// G4 needed classes
class G4Step;
class G4HCofThisEvent;

class ZDC_FiberSD : public G4VSensitiveDetector
{
 FRIEND_TEST( ZDC_FiberSDtest, ProcessHits );
 public:

  ZDC_FiberSD(const G4String& name, const G4String& hitCollectionName, const float &readoutPos);

  // Initialize from G4
  void Initialize(G4HCofThisEvent *) override final;
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;


 private:
  ZDC_SimFiberHitCollectionBuilder* getHitCollection() const;

  std::string m_hitCollectionName;
  // Non-owning cache set by Initialize; HitCollectionMap owns the collection.
  ZDC_SimFiberHitCollectionBuilder* m_HitColl{};
  float m_readoutPos;
};

#endif //ZDC_SD_ZDC_FIBER_SD_H
