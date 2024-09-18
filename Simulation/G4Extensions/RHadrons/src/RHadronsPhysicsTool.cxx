/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "RHadronsPhysicsTool.h"
// package headers
#include "CustomParticleFactory.h"
#include "FullModelHadronicProcess.hh"
#include "RHadronPythiaDecayer.h"
// Geant4 headers
#include "globals.hh"
#include "G4ParticleTable.hh"
#include "G4VProcess.hh"
#include "G4Transportation.hh"
#include "G4hMultipleScattering.hh"
#include "G4hIonisation.hh"
#include "G4ProcessManager.hh"
#include "G4Decay.hh"
#include "G4BaryonConstructor.hh"

// STL headers
#include <string>


//-----------------------------------------------------------------------------
// Implementation file for class : RHadronsPhysicsTool
//
// 2015-05-14 Edoardo Farina
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RHadronsPhysicsTool::RHadronsPhysicsTool( const std::string& type,
                                          const std::string& nam,const IInterface* parent )
  : base_class ( type, nam , parent )
{
  m_physicsOptionType = G4AtlasPhysicsOption::Type::BSMPhysics;
}

//=============================================================================
// Destructor
//=============================================================================

RHadronsPhysicsTool::~RHadronsPhysicsTool()
{
}

//=============================================================================
// Initialize
//=============================================================================
StatusCode RHadronsPhysicsTool::initialize( )
{
  ATH_MSG_DEBUG("RHadronsPhysicsTool::initialize()");
  this->SetPhysicsName(name());
  return StatusCode::SUCCESS;
}

RHadronsPhysicsTool* RHadronsPhysicsTool::GetPhysicsOption()
{
  return this;
}


void RHadronsPhysicsTool::ConstructParticle()
{
  ATH_MSG_DEBUG("RHadronsPhysicsTool::ConstructParticle() - start");
  CustomParticleFactory::loadCustomParticles();
  ATH_MSG_DEBUG("RHadronsPhysicsTool::ConstructParticle() - end");
}
void RHadronsPhysicsTool::ConstructProcess()
{
  ATH_MSG_DEBUG("RHadronsPhysicsTool::ConstructProcess() - start");
  G4Decay* theDecayProcess = new G4Decay();
  theDecayProcess->SetExtDecayer( new RHadronPythiaDecayer("RHadronPythiaDecayer") );
  G4ParticleTable::G4PTblDicIterator* particleIterator = G4ParticleTable::GetParticleTable()->GetIterator();
  particleIterator->reset();

  //First deal with the standard particles that G4 doesn't know about...
  //G4Etac::Definition();
  G4BaryonConstructor::ConstructParticle();
  ATH_MSG_DEBUG("RHadronsPhysicsTool::ConstructProcess() - m_standardpdgidtodecay = " << m_standardpdgidtodecay);
  G4ProcessManager *templateProcessMgr = G4ParticleTable::GetParticleTable()->FindParticle(4122)->GetProcessManager();
  for (const int pid : m_standardpdgidtodecay.value()) {
    ATH_MSG_VERBOSE ( "Adding decay for "<<pid );
    G4ParticleDefinition *particle = G4ParticleTable::GetParticleTable()->FindParticle( pid );
    if (particle) {
      ATH_MSG_VERBOSE ( particle->GetParticleName()<<" is standard for Pythia, lifetime is "<<particle->GetPDGLifeTime() );
      G4ProcessManager *processMgr = particle->GetProcessManager();
      if (!processMgr) {
        ATH_MSG_VERBOSE ( "No process manager found for " << particle->GetParticleName() << " (" << pid << "). Copying process manager from 4122 (one we know works) to this particle" );
        particle->SetProcessManager(new G4ProcessManager(*templateProcessMgr));
        processMgr = particle->GetProcessManager();
      }
      G4ProcessVector *pros = processMgr->GetProcessList();
      for (unsigned int pi=0; pi<pros->size(); ++pi) {
        if ((*pros)[pi]->GetProcessType()==fDecay) {
          ATH_MSG_VERBOSE ( "Found a pre-existing decay process for " <<particle->GetParticleName() << " (" << pid << "). Will remove in favour of using RHadronPythiaDecayer." );
          processMgr->RemoveProcess(pi);
          break;
        }
      }
      for (unsigned int pi=0; pi<pros->size(); ++pi) {
        if ((*pros)[pi]->GetProcessType()==fDecay) {
          ATH_MSG_WARNING ( "There is another decay process for " <<particle->GetParticleName() << " (" << pid << ") already defined!" );
          processMgr ->DumpInfo();
        }
      }
      ATH_MSG_VERBOSE ( "Adding decay process for " <<particle->GetParticleName() << " (" << pid << ") using RHadronPythiaDecayer." );
      processMgr ->AddProcess(theDecayProcess);
      processMgr ->SetProcessOrdering(theDecayProcess, idxPostStep); processMgr ->SetProcessOrdering(theDecayProcess, idxAtRest);
      //processMgr ->DumpInfo();
    } else {
      ATH_MSG_WARNING ( "Particle with pdgid "<<pid<<" has no definition in G4?" );
    }
  } // Loop over all particles that we need to define

  // Now add RHadrons... Keep a vector of those we've already dealt with
  std::vector<int> handled;
  // Use the G4 particle iterator
  while ((*particleIterator)()) {
    G4ParticleDefinition *particle = particleIterator->value();
    if (CustomParticleFactory::isCustomParticle(particle)) {
      if (find(handled.begin(),handled.end(),particle->GetPDGEncoding())==handled.end()) {
        handled.push_back(particle->GetPDGEncoding());
        ATH_MSG_VERBOSE ( particle->GetParticleName() << " (" << particle->GetPDGEncoding() << ") " << " is a Custom Particle. Attempting to add a decay process." );
        G4ProcessManager *processMgr = particle->GetProcessManager();
        if (particle->GetParticleType()=="rhadron"  ||
           particle->GetParticleType()=="mesonino" ||
           particle->GetParticleType()=="sbaryon"  ) {
          processMgr->AddDiscreteProcess(new FullModelHadronicProcess());
          if (theDecayProcess->IsApplicable(*particle)) {
            ATH_MSG_VERBOSE ( "Adding decay..." );
            processMgr ->AddProcess(theDecayProcess);
            // set ordering for PostStepDoIt and AtRestDoIt
            processMgr->SetProcessOrdering(theDecayProcess, idxPostStep);
            processMgr->SetProcessOrdering(theDecayProcess, idxAtRest);
          } else {
            ATH_MSG_WARNING ( "No decay allowed for " << particle->GetParticleName() );
            if (!particle->GetPDGStable() && particle->GetPDGLifeTime()<0.1*CLHEP::ns) {
              ATH_MSG_WARNING ( "Gonna decay it anyway!!!" );
              processMgr->AddProcess(theDecayProcess);
              // set ordering for PostStepDoIt and AtRestDoIt
              processMgr->SetProcessOrdering(theDecayProcess, idxPostStep);
              processMgr->SetProcessOrdering(theDecayProcess, idxAtRest);
            }
          }
        }
        if (particle->GetPDGCharge()/CLHEP::eplus != 0) {
          processMgr->AddProcess(new G4hMultipleScattering, -1, 1, 1);
          processMgr->AddProcess(new G4hIonisation, -1, 2, 2);
          ATH_MSG_VERBOSE ( "Processes for charged particle added." );
        } else {
          ATH_MSG_VERBOSE ( "It is neutral!!" );
        }
        processMgr->DumpInfo();
      } else {
        ATH_MSG_VERBOSE ( "Skipping already handled particle: "<<particle->GetParticleName() );
      } // If it has not been handled yet
    } // If this is one of our custom particles (RHadrons)
  } // End of the particle iterator
  ATH_MSG_DEBUG("RHadronsPhysicsTool::ConstructProcess() - list of handled RHadrons = " << handled);
  ATH_MSG_DEBUG("RHadronsPhysicsTool::ConstructProcess() - end");
}
