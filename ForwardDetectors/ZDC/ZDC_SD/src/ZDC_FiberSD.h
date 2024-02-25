/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_SD_ZDC_FIBER_SD_H
#define ZDC_SD_ZDC_FIBER_SD_H

// Base class
#include "G4VSensitiveDetector.hh"

// use of the hits
#include "ZDC_SimEvent/ZDC_SimFiberHit_Collection.h"
#include "StoreGate/WriteHandle.h"

// STL header
#include <string>
#include <map>
#include <gtest/gtest_prod.h>

// G4 needed classes
class G4Step;
class G4HCofThisEvent;

class ZDC_FiberSD : public G4VSensitiveDetector
{
 FRIEND_TEST( ZDC_FiberSDtest, ProcessHits );
 FRIEND_TEST( ZDC_FiberSDtest, Initialize );
 FRIEND_TEST( ZDC_FiberSDtest, StartOfAthenaEvent );
 FRIEND_TEST( ZDC_FiberSDtest, EndOfAthenaEvent );
 public:

  ZDC_FiberSD(const G4String& name, const G4String& hitCollectionName, const float &readoutPos);
  ~ZDC_FiberSD();

  // Initialize from G4
  void Initialize(G4HCofThisEvent *) override final;
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;
  // Called from ZDC_FiberSDTool::Gather
  void EndOfAthenaEvent();


 private:
  SG::WriteHandle<ZDC_SimFiberHit_Collection> m_HitColl;
  std::map< uint32_t, ZDC_SimFiberHit* > m_hitMap;
  float m_readoutPos;
};

#endif //ZDC_SD_ZDC_FIBER_SD_H
