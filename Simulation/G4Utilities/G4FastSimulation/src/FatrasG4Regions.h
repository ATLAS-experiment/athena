/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FATRASG4REGIONS_H
#define G4FASTSIMULATION_FATRASG4REGIONS_H

#include <string>
#include <vector>

class G4Region;

/** Every Geant4 region with a root volume inside the given envelope volumes,
 the envelopes' own region included. A region reaches every daughter that is
 not the root of another region, so together these cover everything inside the
 envelopes. Needs the geometry to be built. **/
std::vector<G4Region*> fatrasG4RegionsInside(const std::vector<std::string>& envelopeVolumes);

#endif
