/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class PhantomBarrelSD
// Sensitive detector for the phantom calorimeter in combined 2004
//
//************************************************************

#ifndef PHANTOMCALORIMETER_PHANTOMBARRELSD_H
#define PHANTOMCALORIMETER_PHANTOMBARRELSD_H

// Base class
#include "G4VSensitiveDetector.hh"

#include "Identifier/Identifier.h"
#include "TileSimEvent/TileHitVectorCellBuilder.h"

// STL header
#include <string>

class G4Step;
class G4TouchableHistory;

class G4Step;
class G4HCofThisEvent;
class TileTBID;

class PhantomBarrelSD: public G4VSensitiveDetector {
  public:
    PhantomBarrelSD(const std::string& name, const std::string& hitCollectionName);
    ~PhantomBarrelSD() = default;

    static constexpr int NCells = 8;
    using HitVectorBuilder = TileHitVectorCellBuilder<NCells>;

    void Initialize(G4HCofThisEvent*) override final;
    G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

  private:
    const TileTBID* m_tileTBID = nullptr;

    HitVectorBuilder* GetHitCollection();

    Identifier m_id[NCells]={};
    const std::string m_hitCollectionName;
    HitVectorBuilder* m_hitCollection{};
};

#endif  // PHANTOMCALORIMETER_PHANTOMBARRELSD_H
