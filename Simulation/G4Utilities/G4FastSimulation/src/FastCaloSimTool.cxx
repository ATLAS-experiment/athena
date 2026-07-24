/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FastCaloSimTool.h"
#include "FastCaloSimModel.h"

#include "AthenaKernel/RNGWrapper.h"
#include "G4Exception.hh"

#include <cstdlib>

FastCaloSimTool::FastCaloSimTool(const std::string& type, const std::string& name, const IInterface *parent)
: FastSimulationBase(type, name, parent)
{
}

StatusCode FastCaloSimTool::initialize()
{
  ATH_CHECK(FastSimulationBase::initialize());
  ATH_CHECK(m_rndmGenSvc.retrieve());
  ATH_CHECK(m_FastCaloSimParametrizationTool.retrieve());
  if (m_doPunchThrough) {
    ATH_CHECK(m_PunchThroughSimWrapper.retrieve());
  }
  return StatusCode::SUCCESS;
}

StatusCode FastCaloSimTool::BeginOfAthenaEvent(HitCollectionMap&){
  const EventContext& ctx = Gaudi::Hive::currentContext();

  FastCaloSimModel* localFastSimModel = m_fastSimModel.Get();
  if( !localFastSimModel ){
    ATH_MSG_ERROR ("BeginOfAthenaEvent: FastSimModel was never created!");
    return StatusCode::FAILURE;
  }
  localFastSimModel->StartOfAthenaEvent(ctx);

  return StatusCode::SUCCESS;
}

StatusCode FastCaloSimTool::EndOfAthenaEvent(HitCollectionMap&){

  const EventContext& ctx = Gaudi::Hive::currentContext();

  FastCaloSimModel* localFastSimModel = m_fastSimModel.Get();
  if( !localFastSimModel ){
    ATH_MSG_ERROR ("EndOfAthenaEvent: FastSimModel was never created!");
    return StatusCode::FAILURE;
  }
  localFastSimModel->EndOfAthenaEvent(ctx);

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
  auto* model = new FastCaloSimModel(name(), region, m_rndmGenSvc,
                                    m_randomEngineName,
                                    m_CaloCellContainerSDName,
                                    m_FastCaloSimParametrizationTool,
                                    m_PunchThroughSimWrapper,
                                    m_doPunchThrough, this);
  m_fastSimModel.Put(model);
  return model;
}
