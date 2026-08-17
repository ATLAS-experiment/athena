/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "FatrasG4.h"

// FatrasG4 physics models
#include "FatrasG4PhotonConversion.h"

// Geant4 particle includes
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4PionPlus.hh"
#include "G4PionMinus.hh"

//Geant4
#include "G4ParticleTable.hh"
#include "Randomize.hh"
#include "G4TouchableHandle.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4DynamicParticle.hh"
#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4VEmProcess.hh"

#include <cmath>
#include <limits>

// HepMCHelpers include
#include "TruthUtils/HepMCHelpers.h"

// G4 sensitive detector includes
#include "G4SDManager.hh"

#define FATRASG4_DEBUG
#define FATRASG4DOIT_DEBUG


FatrasG4::FatrasG4(const std::string& name,
                         G4Region* region,
                         bool doFlowConversion,
                         FatrasG4Tool * /*FatrasG4Tool*/)

: G4VFastSimulationModel(name, region),
  m_photonConversion(),
  m_generator(*G4Random::getTheEngine()),
  m_doFlowConversion(doFlowConversion)
{
}




G4bool FatrasG4::IsApplicable(const G4ParticleDefinition& particleType)
{
  // Check whether we can simulate the particle with FatrasG4
  bool isPhoton   = &particleType == G4Gamma::GammaDefinition();

  // FatrasG4 is applicable if it is photon, electron, positron or any hadron
  bool isApplicable = isPhoton;

  #ifdef FATRASG4_DEBUG
    const std::string pName = particleType.GetParticleName();
    G4cout<< "[FatrasG4::IsApplicable] Got " << pName <<G4endl;
    if(isApplicable) G4cout<<"[FatrasG4::IsApplicable] APPLICABLE"<<G4endl;
    else G4cout<<"[FatrasG4::IsApplicable] NOT APPLICABLE"<<G4endl;
  #endif

  return isApplicable;
}

G4bool FatrasG4::ModelTrigger(const G4FastTrack& fastTrack)
{

  #ifdef FATRASG4_DEBUG
    G4cout<<"[FatrasG4::ModelTrigger] Got particle with "                                                      <<"\n"
                                    <<" pdg=" <<fastTrack.GetPrimaryTrack() -> GetDefinition()->GetPDGEncoding()  <<"\n"
                                    <<" Ekin="<<fastTrack.GetPrimaryTrack() -> GetKineticEnergy()                 <<"\n"
                                    <<" p="   <<fastTrack.GetPrimaryTrack() -> GetMomentum().mag()                <<"\n"
                                    <<" x="   <<fastTrack.GetPrimaryTrack() -> GetPosition().x()                  <<"\n"
                                    <<" y="   <<fastTrack.GetPrimaryTrack() -> GetPosition().y()                  <<"\n"
                                    <<" z="   <<fastTrack.GetPrimaryTrack() -> GetPosition().z()                  <<"\n"
                                    <<" r="   <<fastTrack.GetPrimaryTrack() -> GetPosition().perp()               <<"\n"
                                    <<" eta=" <<fastTrack.GetPrimaryTrack() -> GetMomentum().eta()                <<"\n"
                                    <<" phi=" <<fastTrack.GetPrimaryTrack() -> GetMomentum().phi()                <<"\n"
                                    <<G4endl;
  #endif
  
  // No conversion until one of the triggers below fires
  m_doConversion = false;

  const G4Track * G4PrimaryTrack = fastTrack.GetPrimaryTrack();
  const G4ParticleDefinition * G4Particle = G4PrimaryTrack -> GetDefinition();

  // Check particle type
  bool isPhoton = G4Particle == G4Gamma::Definition();
  if (!isPhoton) return false;

  // Check particle energy
  // for FatrasG4 we use the fast models for 1-100GeV
  const auto particleEnergy = G4PrimaryTrack -> GetTotalEnergy();
  if (particleEnergy < 1*CLHEP::GeV || particleEnergy > 100*CLHEP::GeV) return false;

  // Decide whether the photon converts in this step. The normalizing flow is
  // trained on Geant4, so it has to be handed the photons Geant4 itself would
  // have converted, while the fast model uses its own ACTS/Fatras path limit.
  return m_doFlowConversion ? G4ConversionTrigger(fastTrack)
                            : ACTSConversionTrigger(fastTrack);

}

bool FatrasG4::G4ConversionTrigger(const G4FastTrack& fastTrack)
{
  // Reproduce the trigger of the Geant4 Bethe-Heitler conversion process the
  // way G4VDiscreteProcess does it: the number of interaction lengths to the
  // next conversion is sampled as -log(u) once per track and decremented by
  // stepLength/meanFreePath at every step, with the mean free path taken from
  // the very same Geant4 process.
  const G4Track * track = fastTrack.GetPrimaryTrack();
  const bool isNewPhoton = isNewPhotonTrack(*track);
  const double stepLength = isNewPhoton ? 0.0 : track -> GetTrackLength() - m_photonPathLength;
  m_photonPathLength = track -> GetTrackLength();

  const double meanFreePath = g4ConversionMeanFreePath(*track);

  if (isNewPhoton) {
    m_nLambdaPhoton = 0.0;
    const double uniform = m_generator.flat();
    m_nLambdaPhotonLimit = (uniform > 0.) ? -std::log(uniform)
                                          : std::numeric_limits<double>::infinity();
  }
  else if (m_photonMeanFreePath > 0.) {
    m_nLambdaPhoton += stepLength / m_photonMeanFreePath;
  }
  m_photonMeanFreePath = meanFreePath;

  m_doConversion = m_nLambdaPhoton >= m_nLambdaPhotonLimit;

  #ifdef FATRASG4_DEBUG
    G4cout<<"[FatrasG4::G4ConversionTrigger] photon trackID="<<track -> GetTrackID()
          <<" nLambda="<<m_nLambdaPhoton<<" / limit="<<m_nLambdaPhotonLimit
          <<" (lambda="<<meanFreePath<<" mm)"<<G4endl;
  #endif

  return m_doConversion;
}

bool FatrasG4::ACTSConversionTrigger(const G4FastTrack& fastTrack)
{
  // The fast model samples the conversion limit in units of radiation length,
  // so the radiation lengths traversed by the photon are accumulated and
  // compared against it. No special case is needed for air: its radiation
  // length suppresses its contribution on its own.
  const G4Track * track = fastTrack.GetPrimaryTrack();
  const bool isNewPhoton = isNewPhotonTrack(*track);
  const double stepLength = isNewPhoton ? 0.0 : track -> GetTrackLength() - m_photonPathLength;
  m_photonPathLength = track -> GetTrackLength();

  const double radLength = track -> GetVolume() -> GetLogicalVolume() -> GetMaterial() -> GetRadlen();

  if (isNewPhoton) {
    m_x0PhotonTraversed = 0.0;
    m_x0Photon = m_photonConversion.generatePathLimits(m_generator, fastTrack).first;
  }
  else if (m_photonRadLength > 0.) {
    m_x0PhotonTraversed += stepLength / m_photonRadLength;
  }
  m_photonRadLength = radLength;

  m_doConversion = m_x0PhotonTraversed >= m_x0Photon;

  #ifdef FATRASG4_DEBUG
    G4cout<<"[FatrasG4::ACTSConversionTrigger] photon trackID="<<track -> GetTrackID()
          <<" X0="<<m_x0PhotonTraversed<<" / limit="<<m_x0Photon<<G4endl;
  #endif

  return m_doConversion;
}

bool FatrasG4::isNewPhotonTrack(const G4Track& track)
{
  // An unseen track ID - or a track length that went backwards - means this is
  // a new photon, for which a fresh conversion limit has to be sampled. The
  // track length check matters because Geant4 track IDs restart at 1 in every
  // event, so an ID comparison alone would let the material budget of one
  // photon leak into the next event.
  const auto trackID = track.GetTrackID();

  if (trackID == m_photonID && track.GetTrackLength() >= m_photonPathLength) return false;

  m_prevPhotonID = m_photonID;
  m_photonID = trackID;

  return true;
}

double FatrasG4::g4ConversionMeanFreePath(const G4Track& track)
{
  // Resolve the Geant4 gamma conversion process on first use. Depending on the
  // physics list "conv" is either a process of its own or a sub-process of
  // G4GammaGeneralProcess, hence the GetEmProcess() lookup on each candidate.
  if (!m_g4ConversionProcessResolved) {
    m_g4ConversionProcessResolved = true;

    const G4ProcessManager * processManager = track.GetDefinition() -> GetProcessManager();
    const G4ProcessVector * processes = processManager ? processManager -> GetProcessList() : nullptr;

    for (std::size_t i = 0; processes && i < processes -> size(); ++i) {
      G4VEmProcess * emProcess = dynamic_cast<G4VEmProcess*>((*processes)[i]);
      if (!emProcess) continue;

      if (emProcess -> GetProcessName() == "conv") {
        m_g4ConversionProcess = emProcess;
        break;
      }
      if (G4VEmProcess * subProcess = emProcess -> GetEmProcess("conv")) {
        m_g4ConversionProcess = subProcess;
        break;
      }
    }

    if (!m_g4ConversionProcess) {
      G4Exception("FatrasG4::g4ConversionMeanFreePath", "FatrasG4NoConvProcess", JustWarning,
                  "Geant4 gamma conversion process not found - FatrasG4 will not convert any photon.");
    }
  }

  if (!m_g4ConversionProcess) return std::numeric_limits<double>::infinity();

  return m_g4ConversionProcess -> MeanFreePath(track);
}

void FatrasG4::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep)
{
    
  // Get Geant4 primary track and information
  const G4Track* G4PrimaryTrack = fastTrack.GetPrimaryTrack();
  const G4ParticleDefinition * G4Particle = G4PrimaryTrack -> GetDefinition();
  const auto pdgEncoding = G4Particle -> GetPDGEncoding();
  const auto trackID = G4PrimaryTrack -> GetTrackID();
  const G4String materialName = G4PrimaryTrack -> GetVolume() -> GetLogicalVolume() -> GetMaterial() -> GetName();

  #ifdef FATRASG4DOIT_DEBUG
    G4cout << "[FatrasG4::DoIt] Material: " << materialName << G4endl;
    G4cout << "                 Particle PDG encoding: " << pdgEncoding << G4endl;
    G4cout << "                 Track ID: " << trackID << G4endl;
  #endif

  // The decision is taken in ModelTrigger only, so there is nothing to do
  // unless one of the conversion triggers has fired
  if (!m_doConversion) return;

  // Consume the flag and force the next ModelTrigger call to treat the photon
  // as a new one. Without this a photon that is not killed below would trigger
  // again at every following step, each time with a zero length step, and the
  // track would never advance.
  m_doConversion = false;
  m_photonID = -999;

  if (m_doFlowConversion) {
    // Normalizing flow conversion, not implemented yet: the photon is handed
    // back to Geant4 untouched
    #ifdef FATRASG4DOIT_DEBUG
      G4cout << "[FatrasG4::DoIt] Flow conversion triggered, but the model is not implemented yet." << G4endl;
    #endif
    return;
  }

  // ACTS/Fatras conversion: creates the electron positron pair and kills the photon
  #ifdef FATRASG4DOIT_DEBUG
    G4cout << "[FatrasG4::DoIt] Running the ACTS photon conversion." << G4endl;
  #endif
  m_photonConversion.run(m_generator, fastTrack, fastStep);

  return;
}

