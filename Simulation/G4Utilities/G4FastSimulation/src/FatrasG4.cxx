/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "FatrasG4.h"

// FatrasG4 physics models
#include "FatrasG4PhotonConversion.h"
#include "FatrasG4ELoss.h"

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

// HepMCHelpers include
#include "TruthUtils/HepMCHelpers.h"

// G4 sensitive detector includes
#include "G4SDManager.hh"

// #define FATRASG4_DEBUG
// #define FATRASG4DOIT_DEBUG
// #define FATRASG4DOIT_DEBUG_PHYSICS
#define FATRASG4_DOCONVERSION

void setTrackParams(G4Track* track, const G4FieldTrack* fieldTrackStep);


FatrasG4::FatrasG4(const std::string& name,
                         G4Region* region,
                         bool doG4Transport,
                         const PublicToolHandle<IG4FatrasTransportTool>& G4FatrasTransportTool,
                         FatrasG4Tool * /*FatrasG4Tool*/)

: G4VFastSimulationModel(name, region),
  m_photonConversion(),
  m_eLoss(),
  m_generator(*G4Random::getTheEngine()),
  m_doG4Transport(doG4Transport),
  m_G4FatrasTransportTool(G4FatrasTransportTool)
{
}




G4bool FatrasG4::IsApplicable(const G4ParticleDefinition& particleType)
{
  // Check whether we can simulate the particle with FatrasG4
  bool isPhoton   = &particleType == G4Gamma::GammaDefinition();
  bool isElectron = &particleType == G4Electron::ElectronDefinition();
  bool isPositron = &particleType == G4Positron::PositronDefinition();
  bool isHadron   = MC::isHadron(particleType.GetPDGEncoding());

  // FatrasG4 is applicable if it is photon, electron, positron or any hadron
  bool isApplicable = isPhoton || isElectron || isPositron || isHadron;

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

  const G4ParticleDefinition * G4Particle = fastTrack.GetPrimaryTrack() -> GetDefinition();
  
  // Check particle type
  bool isPhoton    = G4Particle == G4Gamma::Definition();
  bool isElectron  = G4Particle == G4Electron::Definition();
  bool isPositron  = G4Particle == G4Positron::Definition();

  // Pass all photons, electrons and positrons to FatrasG4
  if (isPhoton || isElectron || isPositron){
    #ifdef FATRASG4_DEBUG
      G4cout<<"[FatrasG4::ModelTrigger] Photons, electrons or positron. Model triggered."<<G4endl;
    #endif
    return true;
  }
  else return false;

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
    G4cout << "[FatrasG4::DoIt] Material at start of transportation: " << materialName << G4endl;
    G4cout << "                 Particle PDG encoding: " << pdgEncoding << G4endl;
    G4cout << "                 Track ID: " << trackID << G4endl;
  #endif

  // Check if it is an electron, positron or photon to apply
  // the appropriate fast model
  const bool isPhoton   = G4Particle == G4Gamma::Definition();
  const bool isElectron = G4Particle == G4Electron::Definition();
  const bool isPositron = G4Particle == G4Positron::Definition();
  // Photons:
  //    - Pair production
  //
  // Electrons:
  //    - Bremsstrahlung emission
  //    - Continuous energy loss
  //    - Multiple scattering
  //
  // Positrons:
  //    - Bremsstrahlung emission
  //    - Continuous energy loss
  //    - Multiple scattering
  //    - Annihilation

  if (isPhoton){
    // Checks for track ID information
    m_photonID = (m_photonID == -999) ? trackID : m_photonID;
    m_photonPathLength = (m_prevPhotonID == -999) ? 0.0 : m_photonPathLength;
    m_x0Photon = (m_prevPhotonID == -999 || m_photonID != m_prevPhotonID) ?
      m_photonConversion.generatePathLimits(m_generator, fastTrack).first : m_x0Photon;
  }
  if (isElectron){
    // Checks for track ID information
    m_electronID = (m_electronID == -999) ? trackID : m_electronID;
    m_electronPathLength = (m_prevElectronID == -999) ? 0.0 : m_electronPathLength;
  }
  if (isPositron){
    // Checks for track ID information
    m_positronID = (m_positronID == -999) ? trackID : m_positronID;
    m_positronPathLength = (m_prevPositronID == -999) ? 0.0 : m_positronPathLength;
  }

  // Do transportation steps on the primary track.
  bool interactionBreak = false;
  auto steps = m_G4FatrasTransportTool -> transport(*G4PrimaryTrack);
  for (unsigned int iStep = 1; iStep < steps.size(); iStep++){

    // Get G4FieldTrack from each step
    const G4FieldTrack& fieldTrack = steps[iStep];
    auto stepLength = (fieldTrack.GetCurveLength() - steps[iStep-1].GetCurveLength());


    #ifdef FATRASG4DOIT_DEBUG
      G4cout << "[FatrasG4::DoIt] Pre-step material name: ";
      G4cout << materialName << G4endl;
      G4cout << "[FatrasG4::DoIt] Step length: ";
      G4cout << stepLength << G4endl;
    #endif

    if (materialName == "std::Air"){
      stepLength = 0.0;
    }

    if (isPhoton){
      m_photonPathLength += stepLength;
      #ifdef FATRASG4DOIT_DEBUG
        G4cout << "[FatrasG4::DoIt] Photon path length: ";
        G4cout << m_photonPathLength << "mm " << G4endl;
      #endif
      #ifdef FATRASG4_DOCONVERSION
      if (m_photonPathLength >= m_x0Photon){
        #ifdef FATRASG4DOIT_DEBUG_PHYSICS
          G4cout << "[FatrasG4::DoIt] Running photon conversion. Photon path length: ";
          G4cout << m_photonPathLength << "mm ";
          G4cout << m_electronPathLength << "mm ";
          G4cout << "Last step length: " << stepLength << "mm ";
          G4cout << "Generated 9/7*X0: " << m_x0Photon << "mm." << G4endl;
        #endif
        m_photonConversion.run(m_generator, fastTrack, fastStep);
        m_photonPathLength = 0.0;

        // Check if fastStep should be updated at 
        // the end of FatrasG4::DoIt
        interactionBreak = true;
        break;
      }
      #endif
    }
    
    if (isElectron){
      m_electronPathLength += stepLength;
      // if (stepLength != 0.0){
      //   interactionBreak = m_eLoss.run(m_generator, stepLength, fastTrack, fastStep);
      //   if (interactionBreak) break;
      // }
      /*
      if (m_electronPathLength >= 100){
        #ifdef FATRASG4DOIT_DEBUG
          G4cout << "[FatrasG4::DoIt] Killing electron. Electron path length: ";
          G4cout << m_electronPathLength << "mm ";
          G4cout << "Last step length: " << stepLength << " mm " << G4endl;

        #endif

        m_electronPathLength = 0.0;
        fastStep.KillPrimaryTrack();
        fastStep.ProposePrimaryTrackPathLength(0.0);
        // Check if fastStep should be updated at 
        // the end of FatrasG4::DoIt
        interactionBreak = true;
        break;
      }
      */
    }

    if (isPositron){
      m_positronPathLength += stepLength;
      // if (stepLength != 0.0){
      //   interactionBreak = m_eLoss.run(m_generator, stepLength, fastTrack, fastStep);
      //   if (interactionBreak) break;
      // }
      /*
      if (m_positronPathLength >= 100){
        #ifdef FATRASG4DOIT_DEBUG
          G4cout << "[FatrasG4::DoIt] Killing positron. Positron path length: ";
          G4cout << m_positronPathLength << "mm ";
          G4cout << m_positronPathLength << "mm ";
          G4cout << "Last step length: " << stepLength << "mm " << G4endl;
        #endif

        m_positronPathLength = 0.0;
        fastStep.KillPrimaryTrack();
        fastStep.ProposePrimaryTrackPathLength(0.0);
        // Check if fastStep should be updated at 
        // the end of FatrasG4::DoIt
        interactionBreak = true;
        break;
      }
      */
    }
  }
  
  if (!interactionBreak){
    // #ifdef FATRASG4DOIT_DEBUG
    //   G4cout << "[FatrasG4::DoIt] Updating fastStep at the end of transport. No interaction happened." << G4endl;;
    // #endif

    const auto& lastStep = steps[steps.size()-1];
    fastStep.ProposePrimaryTrackFinalPosition(lastStep.GetPosition(), false);
    fastStep.ProposePrimaryTrackFinalMomentumDirection(lastStep.GetMomentumDirection(), false);
    // fastStep.ProposePrimaryTrackFinalKineticEnergy(lastStep.GetKineticEnergy());
  }

  // Update particle track IDs at the end of DoIt
  if (isPhoton) m_prevPhotonID = m_photonID;
  if (isElectron) m_prevElectronID = m_electronID;
  if (isPositron) m_prevPositronID = m_positronID;

  return;  
}

