/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILESIMEVENT_TILEHITVECTORCELLBUILDER_H
#define TILESIMEVENT_TILEHITVECTORCELLBUILDER_H

#include "Identifier/Identifier.h"
#include "TileSimEvent/TileHitVector.h"

#include <array>
#include <cstddef>
#include <memory>
#include <string>

/**
 * Event-owned TileHit accumulator for SDs that merge one TileSimHit per cell.
 *
 * The sensitive detector is owned by Geant4, so per-Athena-event merge state
 * should live in the hit collection. Finalization is intentionally delayed
 * until Gather to support multiple G4Events per Athena event and to preserve
 * the legacy output ordering: one merged TileHit per cell, in cell-index order.
 */
template <std::size_t NCells>
class TileHitVectorCellBuilder : public TileHitVector
{
public:
  explicit TileHitVectorCellBuilder(const std::string& collectionName)
    : TileHitVector(collectionName)
  {}

  bool HasHit(std::size_t index) const { return m_hits[index] != nullptr; }

  void AddHit(std::size_t index, Identifier id, double energy, double time = 0.0, double deltaT = 0.0)
  {
    if (m_hits[index]) {
      m_hits[index]->add(energy, time, deltaT);
    } else {
      m_hits[index] = std::make_unique<TileSimHit>(id, energy, time, deltaT);
    }
  }

  void Finalize()
  {
    for (std::size_t index = 0; index < NCells; ++index) {
      if (m_hits[index]) {
        Insert(TileHit(m_hits[index].get()));
        m_hits[index].reset();
      }
    }
  }

private:
  std::array<std::unique_ptr<TileSimHit>, NCells> m_hits{};
};

#endif // TILESIMEVENT_TILEHITVECTORCELLBUILDER_H
