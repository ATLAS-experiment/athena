/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "FatrasG4.h"


// Geant4 particle includes
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4PionPlus.hh"
#include "G4PionMinus.hh"

//Geant4
#include "G4ParticleTable.hh"
#include "Randomize.hh"

// HepMCHelpers include
#include "TruthUtils/HepMCHelpers.h"

// G4 sensitive detector includes
#include "G4SDManager.hh"

//#define FATRASG4_DEBUG


FatrasG4::FatrasG4(const std::string& name,
                         G4Region* region,
                         bool doG4Transport,
                         FatrasG4Tool * /*FatrasG4Tool*/)

: G4VFastSimulationModel(name, region),
  m_doG4Transport(doG4Transport)
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
    
    // VERY trivial  DoIt method to get things work
    // Get Geant4 primary track
    const G4Track * G4PrimaryTrack = fastTrack.GetPrimaryTrack();

#ifdef FATRASG4_DEBUG
    G4cout<<"[FatrasG4::DoIt] Handling particle with Ekin: " << G4PrimaryTrack->GetKineticEnergy() <<" MeV. Killing the primary track."<<G4endl;
#endif
    
   // Just Kill particles below 10 MeV
   if(G4PrimaryTrack -> GetKineticEnergy() < 10){
	   fastStep.KillPrimaryTrack();
   }
   
   return;
    
}

