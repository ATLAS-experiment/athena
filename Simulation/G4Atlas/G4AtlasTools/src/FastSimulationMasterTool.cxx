/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Base class
#include "FastSimulationMasterTool.h"

// For the process setup
#include "G4ParticleTable.hh"
#include "G4FastSimulationManagerProcess.hh"
#include "G4ProcessManager.hh"

FastSimulationMasterTool::FastSimulationMasterTool(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type,name,parent)
{
}

StatusCode FastSimulationMasterTool::initializeFastSims(){
  ATH_MSG_VERBOSE( name() << "::initializeFastSims()" );
  // Loop through list of fast simulations and retrieve them
  //  This forces a call to initialize() for each of them
  ATH_MSG_INFO( "Initializing list of " << m_FastSimList.size() << " fast simulation tools in " << name() );
  CHECK( m_FastSimList.retrieve() );

  // Initialize the FastSim processes. Each process will attach
  // itself to the relevant region at construction
  for (auto& ifs : m_FastSimList){
    CHECK(ifs->initializeFastSim());
  }

  return StatusCode::SUCCESS;
}

StatusCode FastSimulationMasterTool::BeginOfAthenaEvent(){
  // Method that gets called at the beginning of every *athena* event
  for (auto& ifs : m_FastSimList){
    CHECK(ifs->BeginOfAthenaEvent());
  }
  return StatusCode::SUCCESS;
}

StatusCode FastSimulationMasterTool::EndOfAthenaEvent(){
  // Method that gets called at the end of every *athena* event
  for (auto& ifs : m_FastSimList){
    CHECK(ifs->EndOfAthenaEvent());
  }
  return StatusCode::SUCCESS;
}
