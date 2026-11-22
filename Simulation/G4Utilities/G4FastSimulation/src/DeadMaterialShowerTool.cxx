/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DeadMaterialShowerTool.h"

#include "DeadMaterialShower.h"

G4VFastSimulationModel* DeadMaterialShowerTool::makeFastSimModel()
{
  ATH_MSG_DEBUG( "Initializing Fast Sim Model" );

  // Create a fresh Fast Sim Model
  return new DeadMaterialShower(name(), getRegion(), m_highEnergy, m_lowEnergy, m_zcutoff);
}
