/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FastCaloSimTool.h"
#include "FastCaloSim.h"


FastCaloSimTool::FastCaloSimTool(const std::string& type, const std::string& name, const IInterface *parent)
: FastSimulationBase(type, name, parent)
{
}

G4VFastSimulationModel* FastCaloSimTool::makeFastSimModel()
{
  ATH_MSG_DEBUG("Initializing Fast Sim Model");

  // Create the FastCaloSim fast simulation model
  return new FastCaloSim(name(), getRegion(), m_FastCaloSimCaloTransportation, m_FastCaloSimCaloExtrapolation, m_G4CaloTransportTool, m_PunchThroughSimWrapper, m_FastCaloSimSvc, m_CaloCellContainerSDName, m_doG4Transport, m_doPunchThrough, this);
}
