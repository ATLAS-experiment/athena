/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class CombinedScintillatorSD
// Sensitive detector for the Scintillator between LAR & Tile
//
//************************************************************

#ifndef COMBINEDSCINTILLATOR_COMBINEDSCINTILLATORSD_H
#define COMBINEDSCINTILLATOR_COMBINEDSCINTILLATORSD_H

// Base Class
#include "G4VSensitiveDetector.hh"

#include "Identifier/Identifier.h"
#include "TileSimEvent/TileHitVectorCellBuilder.h"

#include <string>

class G4Step;
class G4HCofThisEvent;
class TileTBID;

class CombinedScintillatorSD : public G4VSensitiveDetector
{
public:
  CombinedScintillatorSD(const std::string& name, const std::string& hitCollectionName);
  ~CombinedScintillatorSD() = default;

  static constexpr int NCells = 2;
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

#endif // COMBINEDSCINTILLATOR_COMBINEDSCINTILLATORSD_H
