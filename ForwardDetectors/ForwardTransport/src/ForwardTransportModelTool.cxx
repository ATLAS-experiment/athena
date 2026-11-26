/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ForwardTransportModelTool.h"

#include "ForwardTransportModel.h"

G4VFastSimulationModel* ForwardTransportModelTool::makeFastSimModel()
{
  ATH_MSG_DEBUG( "Initializing Fast Sim Model" );

  int verboseLevel(0);
  if(msgLvl(MSG::VERBOSE)) verboseLevel = 10;
  else if(msgLvl(MSG::DEBUG)) verboseLevel = 5;
  // Create a fresh Fast Sim Model
  return new ForwardTransportModel(name(), getRegion(), verboseLevel, m_FwdTrSvcName);

}
