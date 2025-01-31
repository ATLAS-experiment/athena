/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FastCaloSimTool.h"
#include "FastCaloSim.h"

#include "AthenaKernel/RNGWrapper.h"


FastCaloSimTool::FastCaloSimTool(const std::string& type, const std::string& name, const IInterface *parent)
: FastSimulationBase(type, name, parent)
{
}

StatusCode FastCaloSimTool::BeginOfAthenaEvent(){
  // Get current event context
  const EventContext& ctx = Gaudi::Hive::currentContext();

  if( !getFastSimModel() ){
    ATH_MSG_ERROR ("BeginOfAthenaEvent: FastSimModel was never created!");
    return StatusCode::FAILURE;
  }
  else{
    FastCaloSim *localFastSimModel = dynamic_cast<FastCaloSim*>(getFastSimModel());
    if(!localFastSimModel){
      ATH_MSG_ERROR ("BeginOfAthenaEvent: Failed to cast m_FastSimModel into G4VFastSimulationModel.");
      return StatusCode::FAILURE;
    }
    localFastSimModel->StartOfAthenaEvent(ctx);
  }

  return StatusCode::SUCCESS;
}

StatusCode FastCaloSimTool::EndOfAthenaEvent(){

  // Get current event context
  const EventContext& ctx = Gaudi::Hive::currentContext();

  if( !getFastSimModel() ){
    ATH_MSG_ERROR ("EndOfAthenaEvent: FastSimModel was never created!");
    return StatusCode::FAILURE;
  }
  else{
    FastCaloSim *localFastSimModel = dynamic_cast<FastCaloSim*>(getFastSimModel());
    if(!localFastSimModel){
      ATH_MSG_ERROR ("EndOfAthenaEvent: Failed to cast m_FastSimModel into G4VFastSimulationModel.");
      return StatusCode::FAILURE;
    }
    localFastSimModel->EndOfAthenaEvent(ctx);
  }
  
  return StatusCode::SUCCESS;
}


G4VFastSimulationModel* FastCaloSimTool::makeFastSimModel()
{
  ATH_MSG_DEBUG("Initializing Fast Sim Model");

  // Create the FastCaloSim fast simulation model
  return new FastCaloSim(name(), m_rndmGenSvc, m_randomEngineName,  m_FastCaloSimCaloTransportation, m_FastCaloSimCaloExtrapolation, m_G4CaloTransportTool, m_PunchThroughSimWrapper,  m_FastCaloSimSvc, m_CaloCellContainerSDName, m_doG4Transport, m_doPhotons, m_doElectrons, m_doHadrons, m_AbsEtaMin, m_AbsEtaMax, m_EkinMin, m_EkinMax, m_doEMECFCS, m_doPunchThrough, this);
}
