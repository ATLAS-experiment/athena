/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEGEOG4SD_TILEHITVECTORBUILDER_H
#define TILEGEOG4SD_TILEHITVECTORBUILDER_H

#include <memory>
#include <string>
#include "TileGeoG4SD/TileGeoG4LookupBuilder.hh"
#include "TileSimEvent/TileHitVector.h"

/**
 * This class is needed to make TileGeoG4LookupBuilder local to the Athena event instead of being a thread-local.
 *
 * We want to prepare for the migration to the Geant4 event loop where SDs are instantiated by the Geant4 threads. 
 * In that scenario, any Athena thread can have its Athena events assigned to different Geant4 threads, therefore
 * they should communicate data through the event info.
 *
 * As we still need to support ISF, the scenario where one Athena event is transported over multiple Geant4 events should be supported.
 * Because we want to both support the 1-toN Athena-to-Geant4 event mapping and prepare for migration to the Geant4 event loop (where
 * consecutive G4Event cannot assume that they will execute on the same thread/SD instance), we need to move the inter-event data 
 * out of the SD/Calculator instances and into the hit container, which is managed by Athena and persisted across the entire Athena event.
 * 
 * This is implemented by moving the TileGeoG4LookupBuilder to the hit container, which is passed to Geant4 events belonging 
 * to the same Athena event through the Geant4 event info.
 *
 * Once we can drop the ISF requirement of 1 Athena event to N Geant4 event mapping, this can be greatly simplified: the lookup builder will
 * only be needed by one G4Event before resetting its state. It should be moved back to the SD/Calculator instance, and reset in
 * G4VSensitiveDetector::EndOfEvent, at which point this class can be removed.
 *
 * See MR !80475
*/
class TileHitVectorBuilder : public TileHitVector
{
  public:
    TileHitVectorBuilder(std::string const& collectionName, std::unique_ptr<TileGeoG4LookupBuilder> lookupBuilder) 
    : TileHitVector(collectionName)
    , m_lookupBuilder(std::move(lookupBuilder))
    {}

    void ResetCells() { m_lookupBuilder->ResetCells(this); };

    TileGeoG4LookupBuilder* GetLookupBuilder() { return m_lookupBuilder.get(); }
  
  private:
    std::unique_ptr<TileGeoG4LookupBuilder> m_lookupBuilder;
};

#endif