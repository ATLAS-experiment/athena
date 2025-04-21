/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// local
#include "LucidPhysicsTool.h"

#include <memory>

#include "G4Cerenkov.hh"
#include "G4OpAbsorption.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4OpRayleigh.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4Scintillation.hh"
#include "G4VProcess.hh"
#include "G4Version.hh"

#if G4VERSION_NUMBER > 1029
#define PARTICLEITERATOR (this->GetParticleIterator())
#elif G4VERSION_NUMBER > 1009
#define PARTICLEITERATOR aParticleIterator
#else
#define PARTICLEITERATOR theParticleIterator
#endif

//-----------------------------------------------------------------------------
// Implementation file for class : LucidPhysicsTool
//
// 18-05-2015 : Edoardo Farina
//-----------------------------------------------------------------------------



//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
LucidPhysicsTool::LucidPhysicsTool( const std::string& type,
                                    const std::string& name,const IInterface* parent )
  : AthAlgTool ( type, name , parent )
{
  m_physicsOptionType = G4AtlasPhysicsOption::Type::GlobalProcesses;
  ATH_MSG_INFO("LucidPhysicsTool "<<type<<" "<<name);
  declareInterface< IPhysicsOptionTool >( this ) ;
}

//=============================================================================
// Destructor
//=============================================================================

LucidPhysicsTool::~LucidPhysicsTool()
{
}

//=============================================================================
// Initialize
//=============================================================================
StatusCode LucidPhysicsTool::initialize( )
{
  ATH_MSG_INFO("LucidPhysicsTool initialize( )");
  return StatusCode::SUCCESS;
}

auto LucidPhysicsTool::GetPhysicsOption() -> UPPhysicsConstructor {
  return std::make_unique<LucidPhysicsTool::PhysicsConstructor>(
      name(), this->msgLevel());
}

void LucidPhysicsTool::PhysicsConstructor::ConstructParticle() {}
void LucidPhysicsTool::PhysicsConstructor::ConstructProcess() {
  G4Cerenkov*          theCerenkovProcess      = new G4Cerenkov         ("Cerenkov");
  G4Scintillation*     theScintillationProcess = new G4Scintillation    ("Scintillation");
  G4OpAbsorption*      theOpAbsorptionProcess  = new G4OpAbsorption     ();
  G4OpRayleigh*        theOpRayleighProcess    = new G4OpRayleigh       ();
  G4OpBoundaryProcess* theOpBoundaryProcess    = new G4OpBoundaryProcess();

  theCerenkovProcess->SetVerboseLevel(0);
  theScintillationProcess->SetVerboseLevel(0);
  theOpAbsorptionProcess->SetVerboseLevel(0);
  theOpRayleighProcess->SetVerboseLevel(0);
  theOpBoundaryProcess->SetVerboseLevel(0);

  G4int MaxNumPhotons = 300;

  theCerenkovProcess->SetTrackSecondariesFirst(true);
  theCerenkovProcess->SetMaxNumPhotonsPerStep(MaxNumPhotons);

  theScintillationProcess->SetTrackSecondariesFirst(true);
#if G4VERSION_NUMBER < 1100
  // This should be defaulted to 1. in 11.0 and newer
  theScintillationProcess->SetScintillationYieldFactor(1.);
#endif

  PARTICLEITERATOR->reset();

  while ((*PARTICLEITERATOR)())
    {

      G4ParticleDefinition* particle = PARTICLEITERATOR->value();
      G4ProcessManager*     pmanager = particle->GetProcessManager();
      G4String          particleName = particle->GetParticleName();

      if (theCerenkovProcess->IsApplicable(*particle)) {
        ATH_MSG_DEBUG( " Adding CerenkovProcess to: " << particleName );
        pmanager->AddProcess        (theCerenkovProcess);
        pmanager->SetProcessOrdering(theCerenkovProcess, idxPostStep);
      }

      if (theScintillationProcess->IsApplicable(*particle)) {
        ATH_MSG_DEBUG( " Adding ScintillationProcess to: " << particleName );
        pmanager->AddProcess              (theScintillationProcess);
        pmanager->SetProcessOrderingToLast(theScintillationProcess, idxAtRest);
        pmanager->SetProcessOrderingToLast(theScintillationProcess, idxPostStep);
      }

      if (particleName == "opticalphoton") {

        ATH_MSG_DEBUG(" Adding OpAbsorptionProcess, OpRayleighProcess and OpBoundaryProcess to opticalphoton " );
        pmanager->AddDiscreteProcess(theOpAbsorptionProcess);
        pmanager->AddDiscreteProcess(theOpRayleighProcess);
        pmanager->AddDiscreteProcess(theOpBoundaryProcess);
      }
  }
}
