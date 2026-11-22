/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "NeutronFastSimTool.h"

#include "NeutronFastSim.h"

G4VFastSimulationModel* NeutronFastSimTool::makeFastSimModel()
{
  ATH_MSG_DEBUG( "Initializing Fast Sim Model" );

  // Create a fresh Fast Sim Model
  return new NeutronFastSim(name(), getRegion(), m_trackFastSimSDName,m_etaCut,m_timeCut);
}
