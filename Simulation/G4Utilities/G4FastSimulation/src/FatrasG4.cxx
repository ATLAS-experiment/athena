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
#include "G4EventManager.hh"
#include "G4TrackingManager.hh"

#include <cmath>

// HepMCHelpers include
#include "TruthUtils/HepMCHelpers.h"

// G4 sensitive detector includes
#include "G4SDManager.hh"

// CLHEP units and constants
#include "CLHEP/Units/SystemOfUnits.h"

//#define FATRASG4_DEBUG
//#define FATRASG4DOIT_DEBUG


FatrasG4::FatrasG4(const std::string& name,
                   G4Region* region,
                   const PublicToolHandle<IActsFatrasG4Tool>& ActsFatrasG4Tool,
                   FatrasG4Tool * /*FatrasG4Tool*/)
: G4VFastSimulationModel(name, region),
  m_ActsFatrasG4Tool(ActsFatrasG4Tool),
  m_photonConversion(),
  m_generator(*G4Random::getTheEngine()),
  m_region(region)
{
}

G4bool FatrasG4::IsApplicable(const G4ParticleDefinition& particleType)
{
  // Check whether we can simulate the particle with FatrasG4
  bool isPhoton   = &particleType == G4Gamma::GammaDefinition();
  bool isElectron = &particleType == G4Electron::ElectronDefinition();
  bool isPositron = &particleType == G4Positron::PositronDefinition();

  // No energy check here: Geant4 calls IsApplicable only when the particle
  // type changes and caches the answer, even across events, so a check on the
  // current track would decide for every later photon. ModelTrigger checks it.

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

  // No conversion until the trigger below fires
  m_doConversion = false;

  // Energy region the fast models are used for; outside it Geant4 converts
  const double energy = fastTrack.GetPrimaryTrack() -> GetTotalEnergy();
  if (energy < s_minEnergy || energy > s_maxEnergy) return false;

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

  // Decide whether the photon converts in this step
  return ACTSConversionTrigger(fastTrack);

}

bool FatrasG4::ACTSConversionTrigger(const G4FastTrack& fastTrack)
{
  // The fast model samples the conversion limit in units of radiation length,
  // so the radiation lengths traversed by the photon inside the FatrasG4
  // region are accumulated and compared against it. No special case is needed
  // for air: its radiation length suppresses its contribution on its own.
  const G4Track * track = fastTrack.GetPrimaryTrack();
  const bool isNewPhoton = isNewPhotonTrack(*track);
  const double stepLength = countedStepLength(*track, isNewPhoton);
  m_photonPathLength = track -> GetTrackLength();

  if (isNewPhoton) {
    m_x0PhotonTraversed = 0.0;
    m_x0Photon = m_photonConversion.generatePathLimits(m_generator, fastTrack).first;
  }
  else if (m_photonRadLength > 0.) {
    m_x0PhotonTraversed += stepLength / m_photonRadLength;
  }
  m_photonRadLength = track -> GetVolume() -> GetLogicalVolume() -> GetMaterial() -> GetRadlen();

  // Outside the region only the step just made in the region can fire it
  m_doConversion = (m_lastStepInRegion || stepLength > 0.0) && m_x0PhotonTraversed >= m_x0Photon;

  #ifdef FATRASG4_DEBUG
    G4cout<<"[FatrasG4::ACTSConversionTrigger] photon trackID="<<track -> GetTrackID()
          <<" X0="<<m_x0PhotonTraversed<<" / limit="<<m_x0Photon<<G4endl;
  #endif

  return m_doConversion;
}

double FatrasG4::countedStepLength(const G4Track& track, bool isNewPhoton)
{
  // ModelTrigger is called at the start of every step in the FatrasG4 region
  // and in the bookkeeping regions around it. The previous step is counted
  // only if it was seen starting in the FatrasG4 region: consecutive step
  // numbers guarantee that no unseen step came in between.
  const int stepNumber = track.GetCurrentStepNumber();
  const bool counted = !isNewPhoton && m_lastStepInRegion && stepNumber == m_lastStepNumber + 1;

  m_lastStepNumber = stepNumber;
  m_lastStepInRegion = track.GetVolume() -> GetLogicalVolume() -> GetRegion() == m_region;

  return counted ? track.GetStepLength() : 0.0;
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

void FatrasG4::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep)
{
    
  #ifdef FATRASG4DOIT_DEBUG
    const G4Track* G4PrimaryTrack = fastTrack.GetPrimaryTrack();
    G4cout << "[FatrasG4::DoIt] Material: " << G4PrimaryTrack -> GetVolume() -> GetLogicalVolume() -> GetMaterial() -> GetName() << G4endl;
    G4cout << "                 Particle PDG encoding: " << G4PrimaryTrack -> GetDefinition() -> GetPDGEncoding() << G4endl;
    G4cout << "                 Track ID: " << G4PrimaryTrack -> GetTrackID() << G4endl;
  #endif

  // The decision is taken in ModelTrigger only, so there is nothing to do
  // unless the conversion trigger has fired
  if (!m_doConversion) return;

  // Start photon conversion modelling
  m_doConversion = false;
  m_photonID = -999;

  // ACTS/Fatras conversion: creates the electron positron pair and kills the photon
  #ifdef FATRASG4DOIT_DEBUG
    G4cout << "[FatrasG4::DoIt] Running the ACTS photon conversion." << G4endl;
  #endif
  m_photonConversion.run(m_generator, fastTrack, fastStep);

  return;
}

