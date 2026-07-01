/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class MuonWallSD
// Sensitive detector for the Muon Wall
//
//************************************************************

#ifndef MUONWALL_MUONWALLSD_H
#define MUONWALL_MUONWALLSD_H

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

class MuonWallSD : public G4VSensitiveDetector
{
public:
  MuonWallSD(const std::string& name, const std::string& hitCollectionName, int verbose);
  ~MuonWallSD() = default;

  static constexpr int NCells = 18;
  using HitVectorBuilder = TileHitVectorCellBuilder<NCells>;

  void Initialize(G4HCofThisEvent*) override final;
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override final;

private:
  const TileTBID* m_tileTBID{};

  static const int s_nCellMu = 14;
  static const int s_nCellS = 4;

  HitVectorBuilder* GetHitCollection();

  Identifier m_id[NCells];
  const std::string m_hitCollectionName;
  HitVectorBuilder* m_hitCollection{};

};

#endif // MUONWALL_MUONWALLSD_H
