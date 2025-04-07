/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "G4ScoringProcessTool.h"

#include <memory>
// package headers
#include "G4ParallelWorldScoringProcess.hh"
// Geant4 headers
#include "G4ProcessManager.hh"
#include "G4Version.hh"
//CLHEP headers
#include "CLHEP/Units/SystemOfUnits.h"


//-----------------------------------------------------------------------------
// Implementation file for class : G4ScoringProcessTool
//
// 26-10-2015 : Edoardo Farina
//-----------------------------------------------------------------------------

#if G4VERSION_NUMBER > 1029
#define PARTICLEITERATOR (this->GetParticleIterator())
#elif G4VERSION_NUMBER > 1009
#define PARTICLEITERATOR aParticleIterator
#else
#define PARTICLEITERATOR theParticleIterator
#endif

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
G4ScoringProcessTool::G4ScoringProcessTool( const std::string& type,
                                          const std::string& nam,const IInterface* parent )
  : base_class ( type, nam , parent )
{
  m_physicsOptionType = G4AtlasPhysicsOption::Type::GlobalProcesses;
}

//=============================================================================
// Destructor
//=============================================================================

G4ScoringProcessTool::~G4ScoringProcessTool()
{

}

//=============================================================================
// Initialize
//=============================================================================
StatusCode G4ScoringProcessTool::initialize( )
{
  ATH_MSG_DEBUG("G4ScoringProcessTool initialize( )");
  return StatusCode::SUCCESS;
}

auto G4ScoringProcessTool::GetPhysicsOption() -> UPPhysicsConstructor {
  return std::make_unique<G4ScoringProcessTool::PhysicsConstructor>(name(),
                                                                    msgLevel());
}

void G4ScoringProcessTool::PhysicsConstructor::ConstructParticle() {}

void G4ScoringProcessTool::PhysicsConstructor::ConstructProcess() {
  // Add parallel world scoring process
  ATH_MSG_DEBUG(" ScoringProcessDefinition::ConstructProcess()");

  G4ParallelWorldScoringProcess* theParallelWorldScoringProcess 
      = new G4ParallelWorldScoringProcess("ParaWorldScoringProc");
  theParallelWorldScoringProcess->SetParallelWorld("Scoring");

  PARTICLEITERATOR->reset();
  while( (*PARTICLEITERATOR)() )
  {
    G4ParticleDefinition* particle = PARTICLEITERATOR->value();
    if (!particle->IsShortLived())
    {
      G4ProcessManager* pmanager = particle->GetProcessManager();
      pmanager->AddProcess(theParallelWorldScoringProcess);
      pmanager->SetProcessOrderingToLast(theParallelWorldScoringProcess, idxAtRest);
      pmanager->SetProcessOrdering(theParallelWorldScoringProcess, idxAlongStep, 1);
      pmanager->SetProcessOrderingToLast(theParallelWorldScoringProcess, idxPostStep);
    }
  }
}
