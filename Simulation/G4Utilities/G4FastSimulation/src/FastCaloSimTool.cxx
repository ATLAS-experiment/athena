/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FastCaloSimTool.h"
#include "FastCaloSimModel.h"

#include "G4Exception.hh"

#include <cstdlib>

FastCaloSimTool::FastCaloSimTool(const std::string& type, const std::string& name, const IInterface *parent)
: FastSimulationBase(type, name, parent)
{
}

StatusCode FastCaloSimTool::initialize()
{
  ATH_CHECK(FastSimulationBase::initialize());
  ATH_CHECK(m_FastCaloSimParametrizationTool.retrieve());
  if (m_doPunchThrough) {
    ATH_CHECK(m_PunchThroughSimWrapper.retrieve());
  }
  return StatusCode::SUCCESS;
}

G4VFastSimulationModel* FastCaloSimTool::makeFastSimModel()
{
  ATH_MSG_DEBUG("Initializing Fast Sim Model");

  G4Region* region = getRegion();
  if (!region) {
    G4Exception("FastCaloSimTool", "MissingFastSimulationRegion",
                FatalException,
                "The configured FastCaloSim region does not exist.");
    std::abort();
  }

  // The base constructor registers the model with the configured region.
  auto* model = new FastCaloSimModel(name(), region,
                                    m_CaloCellContainerSDName,
                                    m_FastCaloSimParametrizationTool,
                                    m_PunchThroughSimWrapper,
                                    m_doPunchThrough);
  return model;
}
