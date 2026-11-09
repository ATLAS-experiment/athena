/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FatrasG4Regions.h"

#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4VPhysicalVolume.hh"

#include <set>

std::vector<G4Region*> fatrasG4RegionsInside(const std::vector<std::string>& envelopeVolumes) {
  // Every logical volume inside the envelopes, the envelopes themselves included
  std::set<const G4LogicalVolume*> inside;
  std::vector<const G4LogicalVolume*> toVisit;
  for (const std::string& name : envelopeVolumes) {
    const G4LogicalVolume* volume = G4LogicalVolumeStore::GetInstance()->GetVolume(name, false);
    if (volume) toVisit.push_back(volume);
  }
  while (!toVisit.empty()) {
    const G4LogicalVolume* volume = toVisit.back();
    toVisit.pop_back();
    if (!inside.insert(volume).second) continue;
    for (std::size_t i = 0; i < volume->GetNoDaughters(); ++i) {
      toVisit.push_back(volume->GetDaughter(i)->GetLogicalVolume());
    }
  }

  std::vector<G4Region*> regions;
  for (G4Region* region : *G4RegionStore::GetInstance()) {
    auto root = region->GetRootLogicalVolumeIterator();
    for (std::size_t i = 0; i < region->GetNumberOfRootVolumes(); ++i, ++root) {
      if (inside.count(*root)) {
        regions.push_back(region);
        break;
      }
    }
  }
  return regions;
}
