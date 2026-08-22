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
#include "G4EventManager.hh"
#include "G4TrackingManager.hh"

#include <cmath>
#include <limits>

// HepMCHelpers include
#include "TruthUtils/HepMCHelpers.h"

// G4 sensitive detector includes
#include "G4SDManager.hh"

// PathResolver
#include "PathResolver/PathResolver.h"

// Geant4 classes used by the normalizing flow conversion
#include "G4VEmModel.hh"
#include "G4Element.hh"
#include "G4MaterialCutsCouple.hh"

// CLHEP units and constants
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Units/PhysicalConstants.h"

#include <algorithm>
#include <cfloat>
#include <filesystem>
#include <numbers>
#include <stdexcept>

#define FATRASG4_DEBUG
#define FATRASG4DOIT_DEBUG


FatrasG4::FatrasG4(const std::string& name,
                         G4Region* region,
                         bool doFlowConversion,
                         const std::string& flowConversionModelPath,
                         FatrasG4Tool * /*FatrasG4Tool*/)

: G4VFastSimulationModel(name, region),
  m_photonConversion(),
  m_generator(*G4Random::getTheEngine()),
  m_doFlowConversion(doFlowConversion)
{
  // The model is only needed by the normalizing flow conversion, so it is not
  // loaded when the ACTS fast model is used
  if (m_doFlowConversion) initializeFlowModel(flowConversionModelPath);
}

void FatrasG4::initializeFlowModel(const std::string& flowConversionModelPath)
{
  // An absolute path is taken as given, deliberately without asking the
  // PathResolver: it logs an ERROR for absolute names, and the job transforms
  // treat any ERROR in the log as fatal, so a perfectly good local model would
  // otherwise fail the job. Anything else is a calibration-area-relative name
  // and is looked up along CALIBPATH.
  std::string resolvedModelPath;
  if (std::filesystem::path(flowConversionModelPath).is_absolute()) {
    if (std::filesystem::is_regular_file(flowConversionModelPath)) {
      resolvedModelPath = flowConversionModelPath;
    }
  }
  else {
    resolvedModelPath = PathResolverFindCalibFile(flowConversionModelPath);
  }

  if (resolvedModelPath.empty()) {
    const std::string message = "Normalizing flow photon conversion model '"
                              + flowConversionModelPath
                              + "' not found - falling back to the ACTS photon conversion.";
    G4Exception("FatrasG4::initializeFlowModel", "FatrasG4NoFlowModel", JustWarning, message.c_str());
    m_doFlowConversion = false;
    return;
  }

  try {
    m_conversionFlow = std::make_unique<FatrasG4ConversionFlowInference>(resolvedModelPath);
  }
  catch (const std::exception& e) {
    // Ort::Exception derives from std::exception, so this covers a missing or
    // malformed graph as well as a bad constants file
    const std::string message = "Could not load the normalizing flow photon conversion model from '"
                              + resolvedModelPath + "': " + e.what()
                              + " - falling back to the ACTS photon conversion.";
    G4Exception("FatrasG4::initializeFlowModel", "FatrasG4BadFlowModel", JustWarning, message.c_str());
    m_conversionFlow.reset();
    m_doFlowConversion = false;
    return;
  }

  #ifdef FATRASG4_DEBUG
    G4cout<<"[FatrasG4::initializeFlowModel] Loaded conversion flow from "
          <<resolvedModelPath<<G4endl;
  #endif
}

G4bool FatrasG4::IsApplicable(const G4ParticleDefinition& particleType)
{
  // Check whether we can simulate the particle with FatrasG4
  bool isPhoton   = &particleType == G4Gamma::GammaDefinition();
  bool isElectron = &particleType == G4Electron::ElectronDefinition();
  bool isPositron = &particleType == G4Positron::PositronDefinition();

  // Check particle energy
  // for FatrasG4 we use the fast models for 1-100GeV
  // IsApplicable is handed a particle type without a track, so the energy is
  // taken from the track Geant4 is currently tracking: that is the very track
  // this call is about, as Geant4 calls IsApplicable at each of its steps.
  const G4TrackingManager * trackingManager = G4EventManager::GetEventManager() -> GetTrackingManager();
  const G4Track * currentTrack = trackingManager ? trackingManager -> GetTrack() : nullptr;
  const auto particleEnergy = currentTrack ? currentTrack -> GetTotalEnergy() : 0.;
  if (particleEnergy < s_minEnergy || particleEnergy > s_maxEnergy) return false;

  // The model only acts on photons. Electrons and positrons are declared
  // applicable so that Geant4 attaches the fast simulation process to them in
  // the region as well; ModelTrigger then always declines them, leaving their
  // transport to the standard physics.
  bool isApplicable = isPhoton || isElectron || isPositron;

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
  // IsApplicable also accepts electrons and positrons, so that Geant4 attaches
  // the fast simulation process to them inside the region. The model never acts
  // on them, so they are declined before anything else is done.
  const G4ParticleDefinition* definition =
      fastTrack.GetPrimaryTrack() -> GetDefinition();
  if (definition == G4Electron::ElectronDefinition() ||
      definition == G4Positron::PositronDefinition())
    return false;

  // No conversion until one of the triggers below fires
  m_doConversion = false;

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

double FatrasG4::selectTargetZ(const G4Track& track)
{
  // The flow is conditioned on the atomic number of the element the photon
  // converts on, so this has to be the cross-section-weighted choice rather
  // than, say, an atom-count average: pair production goes roughly as Z^2, and
  // the inner detector is full of composites. Reusing the conversion process
  // already resolved for the mean free path gets exactly the element selectors
  // the physics list built, with no second model to initialise.
  g4ConversionMeanFreePath(track);
  if (!m_g4ConversionProcess) return 0.;

  const G4MaterialCutsCouple * couple = track.GetMaterialCutsCouple();
  if (!couple) return 0.;

  // SelectModelForMaterial is the public form of SelectModel: same model
  // manager lookup, it just leaves the process's current couple alone, which
  // does not matter here because SelectRandomAtom is handed the couple anyway.
  const double energy = track.GetKineticEnergy();
  G4VEmModel * model = m_g4ConversionProcess -> SelectModelForMaterial(energy, couple -> GetIndex());
  if (!model) return 0.;

  const G4Element * element =
      model -> SelectRandomAtom(couple, track.GetDefinition(), energy, 0., DBL_MAX);

  return element ? element -> GetZ() : 0.;
}

void FatrasG4::runFlowConversion(const G4FastTrack& fastTrack, G4FastStep& fastStep)
{
  const G4Track * track = fastTrack.GetPrimaryTrack();
  const double eGamma = track -> GetTotalEnergy();
  const double targetZ = selectTargetZ(*track);

  // The one call this is all for: one graph run gives the pair kinematics
  const FatrasG4ConversionFlowSample sampled =
      m_conversionFlow -> sample(eGamma / CLHEP::MeV, targetZ, m_generator);

  // The flow gives kinetic energies and the leading lepton's polar angle only.
  // The sub-leading one is placed coplanar and opposite so that transverse
  // momentum balances, and the azimuth is uniform.
  const G4ThreeVector gammaDirection = track -> GetMomentumDirection().unit();
  const G4ThreeVector xAxis = gammaDirection.orthogonal().unit();
  const G4ThreeVector yAxis = gammaDirection.cross(xAxis);
  auto direction = [&](double theta, double phi) {
    return std::sin(theta) * std::cos(phi) * xAxis +
           std::sin(theta) * std::sin(phi) * yAxis +
           std::cos(theta) * gammaDirection;
  };
  auto momentum = [](double kineticEnergy) {
    return std::sqrt(kineticEnergy * (kineticEnergy + 2. * CLHEP::electron_mass_c2));
  };

  const double eLead = sampled.eLead * CLHEP::MeV;
  const double eSub = sampled.eSub * CLHEP::MeV;

  const double phiLead = 2. * std::numbers::pi * m_generator.flat();
  const double pLead = momentum(eLead);
  const double pSub = momentum(eSub);
  const double sinSub =
      std::clamp((pSub > 0.) ? pLead * std::sin(sampled.thetaLead) / pSub : 0., -1., 1.);

  const G4ThreeVector dirLead = direction(sampled.thetaLead, phiLead);
  const G4ThreeVector dirSub = direction(std::asin(sinSub), phiLead + std::numbers::pi);

  // The pair is sorted by energy, not by charge, so the label is drawn here.
  const bool leadIsElectron = m_generator.flat() < 0.5;
  const double eElectron = leadIsElectron ? eLead : eSub;
  const double ePositron = leadIsElectron ? eSub : eLead;
  const G4ThreeVector& dirElectron = leadIsElectron ? dirLead : dirSub;
  const G4ThreeVector& dirPositron = leadIsElectron ? dirSub : dirLead;

  const G4ThreeVector vertex = track -> GetPosition();
  const double time = track -> GetGlobalTime();

  fastStep.SetNumberOfSecondaryTracks(2);
  fastStep.CreateSecondaryTrack(G4DynamicParticle(G4Electron::Definition(), dirElectron, eElectron),
                                vertex, time, false);
  fastStep.CreateSecondaryTrack(G4DynamicParticle(G4Positron::Definition(), dirPositron, ePositron),
                                vertex, time, false);

  // The photon is replaced by its pair
  fastStep.KillPrimaryTrack();
  fastStep.ProposePrimaryTrackPathLength(0.0);

  #ifdef FATRASG4DOIT_DEBUG
    G4cout << "[FatrasG4::runFlowConversion] E=" << eGamma << " MeV on Z=" << targetZ
           << (sampled.isTriplet ? " (triplet)" : " (nuclear)")
           << ": e-=" << eElectron << " e+=" << ePositron
           << " theta=" << sampled.thetaLead << " recoil=" << sampled.eRecoil << G4endl;
  #endif
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

  // Start photon conversion modelling
  m_doConversion = false;
  m_photonID = -999;

  // Run normalizing flow model
  if (m_doFlowConversion) {
    #ifdef FATRASG4DOIT_DEBUG
      G4cout << "[FatrasG4::DoIt] Running the normalizing flow photon conversion." << G4endl;
    #endif
    runFlowConversion(fastTrack, fastStep);
    return;
  }

  // ACTS/Fatras conversion: creates the electron positron pair and kills the photon
  #ifdef FATRASG4DOIT_DEBUG
    G4cout << "[FatrasG4::DoIt] Running the ACTS photon conversion." << G4endl;
  #endif
  m_photonConversion.run(m_generator, fastTrack, fastStep);

  return;
}

