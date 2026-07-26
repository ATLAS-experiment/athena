/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FATRASG4_H
#define G4FASTSIMULATION_FATRASG4_H

#include "G4VFastSimulationModel.hh"
#include "FatrasG4Tool.h"
#include "FatrasG4PhotonConversion.h"
#include "Randomize.hh"

class G4FieldTrack;
class G4SafetyHelper;

class FatrasG4 : public G4VFastSimulationModel
{
public:
  FatrasG4(const std::string& name,
           G4Region* region,
           FatrasG4Tool* FatrasG4Tool);

  ~FatrasG4() = default;

  G4bool IsApplicable(const G4ParticleDefinition&) override final;
  void DoIt(const G4FastTrack&, G4FastStep&) override final;

  /** Determines the applicability of the fast sim model to this particular track.
  Checks that geometric location, energy, and particle type are within bounds **/
  G4bool ModelTrigger(const G4FastTrack &) override final;

private:
  // Photon conversion model
  FatrasG4PhotonConversion m_photonConversion;
  // RNG engine
  CLHEP::HepRandomEngine& m_generator;

  // Particle path length
  double m_photonPathLength = 0.0;
  double m_electronPathLength = 0.0;
  double m_positronPathLength = 0.0;

  // G4Track IDs
  int m_photonID = -999;
  int m_electronID = -999;
  int m_positronID = -999;

  // Extra G4Track IDs for checks
  int m_prevPhotonID = -999;
  int m_prevElectronID = -999;
  int m_prevPositronID = -999;

  // Particle radiation length
  double m_x0Photon = 0.0;
};

#endif
