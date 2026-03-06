/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FATRASG4_H
#define G4FASTSIMULATION_FATRASG4_H

// Service handle to define smart pointer to Fatras parametrisation service
#include "GaudiKernel/ServiceHandle.h"
// Geant4 fast simulation base class
#include "G4VFastSimulationModel.hh"

class FatrasG4: public G4VFastSimulationModel
{
 public:
  FatrasG4(const std::string& name,
	   G4Region* region);
  
  virtual ~FatrasG4() = default;

  virtual G4bool IsApplicable(const G4ParticleDefinition&) override final;
  virtual void DoIt(const G4FastTrack&, G4FastStep&) override final;

  /** Determines the applicability of the fast sim model to this particular track.
  Checks that geometric location, energy, and particle type are within bounds **/
  virtual G4bool ModelTrigger(const G4FastTrack &) override final;

 private:

};

#endif //G4FASTSIMULATION_FATRASG4_H

