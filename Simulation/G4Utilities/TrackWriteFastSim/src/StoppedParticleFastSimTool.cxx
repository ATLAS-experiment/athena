/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "StoppedParticleFastSimTool.h"
#include "StoppedParticleFastSim.h"

StatusCode StoppedParticleFastSimTool::initialize()
{
  CHECK(m_trackFastSimSDTool.retrieve());
  CHECK(m_trackFastSimSDTool->initializeSD());
  return StatusCode::SUCCESS;
}

G4VFastSimulationModel* StoppedParticleFastSimTool::makeFastSimModel()
{
  ATH_MSG_DEBUG( "Initializing Fast Sim Model" );

  // Create a fresh Fast Sim Model
  return new StoppedParticleFastSim(name(), getRegion(), m_trackFastSimSDTool->name());
}
