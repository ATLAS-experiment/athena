/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArFcalDegCondAlg.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "StoreGate/WriteCondHandle.h"
#include "StoreGate/ReadCondHandle.h"

StatusCode LArFcalDegCondAlg::initialize() {
 
  ATH_CHECK(m_factorsKey.initialize());

  ATH_CHECK(m_outputKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode LArFcalDegCondAlg::execute(const EventContext& ctx) const {

  ATH_MSG_DEBUG("executing");

  SG::WriteCondHandle<LArFcalDegFactors> outputHdl{m_outputKey,ctx};
  
  if (outputHdl.isValid()) {
    ATH_MSG_DEBUG("Found valid write handle");
    return StatusCode::SUCCESS;
  }  

  SG::ReadCondHandle<CondAttrListCollection> factorsHdl(m_factorsKey, ctx);
  if (!factorsHdl.isValid()) {
    ATH_MSG_ERROR("Do not have degradation factors map !!!!");
    return StatusCode::FAILURE;
  }
  outputHdl.addDependency(factorsHdl);

  std::unique_ptr<LArFcalDegFactors> output=std::make_unique<LArFcalDegFactors>(*factorsHdl, msg());

  if(output->get().empty() || !output->isValid()) {
    ATH_MSG_ERROR("Empty or unreadable degradation factors blob in " << m_factorsKey.key() << " !!!!");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("FCal1 degradation table: " << output->nChannels() << " channels x "
               << LArFcalDegFactors::muGrid.size() << " mu points");

  if(outputHdl.record(std::move(output)).isFailure()) {
    ATH_MSG_ERROR("Could not record LArFcalDegFactors object with " 
                  << outputHdl.key() 
                  << " with EventRange " << outputHdl.getRange()
                  << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new " << outputHdl.key() << " with range " << outputHdl.getRange() << " into Conditions Store");

  return StatusCode::SUCCESS;
}
