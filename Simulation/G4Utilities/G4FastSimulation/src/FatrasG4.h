/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FATRASG4_H
#define G4FASTSIMULATION_FATRASG4_H

#include "G4VFastSimulationModel.hh"
#include "FatrasG4Tool.h"
#include "FatrasG4PhotonConversion.h"
#include "Randomize.hh"

#include <limits>

class G4FieldTrack;
class G4SafetyHelper;
class G4Track;
class G4VEmProcess;

class FatrasG4 : public G4VFastSimulationModel
{
public:
  FatrasG4(const std::string& name,
           G4Region* region,
           bool doFlowConversion,
           FatrasG4Tool* FatrasG4Tool);

  ~FatrasG4() = default;

  G4bool IsApplicable(const G4ParticleDefinition&) override final;
  void DoIt(const G4FastTrack&, G4FastStep&) override final;

  /** Determines the applicability of the fast sim model to this particular track.
  Checks that geometric location, energy, and particle type are within bounds **/
  G4bool ModelTrigger(const G4FastTrack &) override final;

private:
  /** Photon conversion trigger following the Geant4 Bethe-Heitler conversion
  process. Used together with the normalizing flow, which is trained on Geant4.
  Sets and returns m_doConversion. **/
  bool G4ConversionTrigger(const G4FastTrack& fastTrack);

  /** Photon conversion trigger following the ACTS/Fatras fast model.
  Sets and returns m_doConversion. **/
  bool ACTSConversionTrigger(const G4FastTrack& fastTrack);

  /** True the first time this photon track is seen. Updates the stored track
  IDs, but leaves m_photonPathLength untouched so that the callers can still
  take a step length difference from it. **/
  bool isNewPhotonTrack(const G4Track& track);

  /** Mean free path of the Geant4 gamma conversion process for this track.
  Returns infinity if the process cannot be found. **/
  double g4ConversionMeanFreePath(const G4Track& track);

  // Photon conversion model
  FatrasG4PhotonConversion m_photonConversion;
  // RNG engine
  CLHEP::HepRandomEngine& m_generator;
  // Boolean flag to enable the normalizing flow photon conversion
  bool m_doFlowConversion;

  // Set by the conversion triggers when the photon converts in the current
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

  // Number of interaction lengths of the Geant4 Bethe-Heitler conversion
  // process traversed by the photon, and the limit sampled for it. Used to
  // trigger the normalizing flow where Geant4 itself would have converted.
  double m_nLambdaPhoton = 0.0;
  double m_nLambdaPhotonLimit = std::numeric_limits<double>::infinity();
  // Mean free path of the conversion process in the previous step
  double m_photonMeanFreePath = 0.0;

  // Geant4 gamma conversion process, resolved lazily on first use
  G4VEmProcess* m_g4ConversionProcess = nullptr;
  bool m_g4ConversionProcessResolved = false;
};

#endif
