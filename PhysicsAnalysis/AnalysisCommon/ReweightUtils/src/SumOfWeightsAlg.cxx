/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// ReweightUtils includes
#include "SumOfWeightsAlg.h"

#include "CxxUtils/checker_macros.h"
#include "ReweightUtils/WeightToolBase.h"
#include "xAODCutFlow/CutBookkeeper.h"

#include <string>

//**********************************************************************

StatusCode SumOfWeightsAlg::initialize ATLAS_NOT_THREAD_SAFE () {
  //                                   ^ due to registerTopFilter

  ATH_MSG_DEBUG("Initializing " << name() << "...");
  ATH_MSG_DEBUG("Retrieving tools...");
  ATH_CHECK(m_weightTools.retrieve());

  ATH_MSG_DEBUG("Tool retrieval completed.");
  unsigned int ntool = m_weightTools.size();
  ATH_MSG_DEBUG("  Tool count: " << ntool);
  const std::string allStreamsStr{"AllStreams"};
  for (size_t itool = 0; itool < ntool; ++itool ) {
    ATH_MSG_DEBUG("    " << m_weightTools[itool]->name());
    if(msgLvl(MSG::DEBUG)) m_weightTools[itool]->print();
    // Get the tool's message property:
    const WeightToolBase* tool = dynamic_cast< const WeightToolBase* >( m_weightTools[ itool ].operator->() );
    if( ! tool ) {
       ATH_MSG_ERROR( "The received tool is not an WeightToolBase?!?" );
       return StatusCode::FAILURE;
    }
    //strip the 'ToolSvc.' off the weight name
    std::string toolName = m_weightTools[itool]->name();
    if(toolName.starts_with ("ToolSvc.")) toolName.replace(0,8,"");
    CutIdentifier cID = m_cutFlowSvc->registerTopFilter( toolName,
                                                         toolName, // description (can be improved FIXME)
                                                         xAOD::CutBookkeeper::CutLogic::ALLEVENTSPROCESSED,
                                                         allStreamsStr,
                                                         true);
    m_cutIDs.push_back(cID);
  }

  return StatusCode::SUCCESS;
}

//**********************************************************************

StatusCode SumOfWeightsAlg::execute(const EventContext& /* ctx */) const {
  ATH_MSG_DEBUG ("Executing " << name() << ", will loop over WeightTools...");

  for (std::size_t i = 0; i < m_cutIDs.size(); ++i) {
    float weight = m_weightTools[i]->getWeight();
    ATH_MSG_DEBUG("   got weight = " << weight << " for " << m_weightTools[i]->name());
    const CutIdentifier cutID = m_cutIDs[i];
    m_cutFlowSvc->addEvent(cutID, weight);
  }

  return StatusCode::SUCCESS;
}
