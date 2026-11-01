/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4HepEmOptionsHelper_h
#define G4HepEmOptionsHelper_h 1

#include "G4PhysicsLists/G4HepEmOptions.h"

#include "G4HepEmConfig.hh"
#include "G4HepEmParameters.hh"

#include "G4Exception.hh"
#include "G4RegionStore.hh"

/// Apply the ATLAS G4HepEm options to the configuration of a G4HepEm
/// tracking manager. Must be called right after the tracking manager is
/// constructed, i.e. in ConstructProcess, before the run initialisation.
inline void ApplyG4HepEmOptions(G4HepEmConfig& config, const G4HepEmOptions& opts)
{
  // Woodcock tracking of photons
  config.GetWoodcockTrackingRegionNames().clear();
  for (const std::string& region : opts.woodcockTrackingRegions) {
    config.SetWoodcockTrackingRegion(region);
  }

  // e+ correction to the theta0 in the Urban MSC
  config.GetG4HepEmParameters()->fIsMSCPositronCor = opts.mscPositronCorrection;

  // e-/e+ energy loss fluctuation: global value, then per-region overrides
  config.SetEnergyLossFluctuation(opts.energyLossFluctuation);
  for (const std::string& region : opts.noEnergyLossFluctuationRegions) {
    // G4HepEmConfig exits without context on unknown regions, so check first
    if (G4RegionStore::GetInstance()->GetRegion(region, false) == nullptr) {
      G4ExceptionDescription msg;
      msg << "Unknown G4Region '" << region
          << "' requested in HepEmNoELossFluctRegions";
      G4Exception("ApplyG4HepEmOptions", "UnknownRegion", FatalException, msg);
    }
    config.SetEnergyLossFluctuation(false, region);
  }

  // Multiple steps in the combined MSC+transportation
  config.SetMultipleStepsInMSCWithTransportation(opts.multipleStepsInMSCTransport);
}

#endif
