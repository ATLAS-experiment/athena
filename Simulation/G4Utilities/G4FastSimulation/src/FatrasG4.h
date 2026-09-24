/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FATRASG4_H
#define G4FASTSIMULATION_FATRASG4_H

#include "G4VFastSimulationModel.hh"

// Geant4 ACTSFatras G4 Tool
#include "G4AtlasInterfaces/IActsFatrasG4Tool.h"

// FatrasG4 tool
#include "FatrasG4Tool.h"

#include "FatrasG4PhotonConversion.h"
#include "Randomize.hh"

#include "CLHEP/Units/SystemOfUnits.h"

#include <string>

class G4FieldTrack;
class G4SafetyHelper;
class G4Track;
class G4Region;

class FatrasG4 : public G4VFastSimulationModel
{
public:
  FatrasG4(const std::string& name,
           G4Region* region,
           const PublicToolHandle<IActsFatrasG4Tool>& ActsFatrasG4Tool,
           FatrasG4Tool * FatrasG4Tool);

  ~FatrasG4() = default;

  G4bool IsApplicable(const G4ParticleDefinition&) override final;
  void DoIt(const G4FastTrack&, G4FastStep&) override final;

  /** Determines the applicability of the fast sim model to this particular track.
  Checks that geometric location, energy, and particle type are within bounds **/
  G4bool ModelTrigger(const G4FastTrack &) override final;

private:
  /** Photon conversion trigger following the ACTS/Fatras fast model.
  Sets and returns m_doConversion. **/
  bool ACTSConversionTrigger(const G4FastTrack& fastTrack);

  /** True the first time this photon track is seen. Updates the stored track
  IDs, but leaves m_photonPathLength untouched so that the callers can still
  take a step length difference from it. **/
  bool isNewPhotonTrack(const G4Track& track);

  /** Length of the previous step if it was made inside the FatrasG4 region,
  0 otherwise. Updates m_lastStepNumber and m_lastStepInRegion for the step
  about to start. Used by the ACTS trigger. **/
  double countedStepLength(const G4Track& track, bool isNewPhoton);

  // Energy region the fast models are used for. Checked in ModelTrigger.
  static constexpr double s_minEnergy = 1.*CLHEP::GeV;
  static constexpr double s_maxEnergy = 100.*CLHEP::GeV;

  // Geant4 ACTSFatras G4 Tool
  PublicToolHandle<IActsFatrasG4Tool> m_ActsFatrasG4Tool;

  // Photon conversion model
  FatrasG4PhotonConversion m_photonConversion;
  // RNG engine
  CLHEP::HepRandomEngine& m_generator;
  // Set by the conversion trigger when the photon converts in the current
  // step, so that DoIt knows what it has been called for
  bool m_doConversion = false;

  // Particle path length. For the photon this is the track length seen at the
  // previous ModelTrigger call, from which the step length is derived.
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

  // Particle radiation length. Conversion limit in units of X0, sampled once
  // per photon by the fast model FatrasG4PhotonConversion.
  double m_x0Photon = 0.0;

  // Radiation lengths the photon has traversed so far. It converts once this
  // exceeds m_x0Photon.
  double m_x0PhotonTraversed = 0.0;
  // Radiation length of the material the photon traversed in the previous step
  double m_photonRadLength = 0.0;

  // Region FatrasG4 is valid in. The model is also attached to bookkeeping
  // regions, where it only counts steps.
  const G4Region* m_region;

  // Step number and region of the step at the last ModelTrigger call
  int m_lastStepNumber = -1;
  bool m_lastStepInRegion = false;
};

#endif
