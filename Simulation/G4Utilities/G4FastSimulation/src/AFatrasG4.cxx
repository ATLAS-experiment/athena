/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "AFatrasG4.h"


// Geant4 particle includes
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"

//Geant4
#include "G4ParticleTable.hh"
#include "Randomize.hh"

// HepMCHelpers include
#include "TruthUtils/HepMCHelpers.h"

// G4 sensitive detector includes
#include "G4SDManager.hh"

//#define AFATRASG4_DEBUG


AFatrasG4::AFatrasG4(const std::string& name,
                         G4Region* region,
                         const PublicToolHandle<IActsFatrasG4Tool>& ActsFatrasG4Tool,
                         bool doG4Transport,
                         AFatrasG4Tool * /*AFatrasG4Tool*/)

: G4VFastSimulationModel(name, region),
  m_ActsFatrasG4Tool(ActsFatrasG4Tool),
  m_doG4Transport(doG4Transport)
{
}

G4bool AFatrasG4::IsApplicable(const G4ParticleDefinition& particleType)
{
  // Check whether we can simulate the particle with FatrasG4
  bool isPhoton   = &particleType == G4Gamma::GammaDefinition();
  bool isElectron = &particleType == G4Electron::ElectronDefinition();
  bool isPositron = &particleType == G4Positron::PositronDefinition();
  bool isHadron   = MC::isHadron(particleType.GetPDGEncoding());

  // FatrasG4 is applicable if it is photon, electron, positron or any hadron
  bool isApplicable = isPhoton || isElectron || isPositron || isHadron;

  #ifdef AFATRASG4_DEBUG
    const std::string pName = particleType.GetParticleName();
    G4cout<< "[AFatrasG4::IsApplicable] Got " << pName <<G4endl;
    if(isApplicable) G4cout<<"[AFatrasG4::IsApplicable] APPLICABLE"<<G4endl;
    else G4cout<<"[AFatrasG4::IsApplicable] NOT APPLICABLE"<<G4endl;
  #endif


  return isApplicable;
}

G4bool AFatrasG4::ModelTrigger(const G4FastTrack& fastTrack)
{

  #ifdef AFATRASG4_DEBUG
    G4cout<<"[AFatrasG4::ModelTrigger] Got particle with "                                                         <<"\n"
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
    #ifdef AFATRASG4_DEBUG 
      G4cout<<"[AFatrasG4::ModelTrigger] Photons, electrons or positron. Model triggered."<<G4endl;
    #endif
    return true;
  }
  else return false;

}

void AFatrasG4::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep)
{
  #ifdef AFATRASG4_DEBUG 
    G4cout<<"AFatrasG4::DoIt called"<<G4endl;
  #endif

  if (!m_ActsFatrasG4Tool.isValid()) {
      G4cerr << "ActsFatrasG4Tool not valid!" << G4endl;
      fastStep.KillPrimaryTrack();
      return;
  }

  m_ActsFatrasG4Tool->simulateFatrasTrack(fastTrack, fastStep);
}


