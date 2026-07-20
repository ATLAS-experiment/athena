/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "StripDigitization.h"
#include "PileUpTools/IPileUpTool.h"

namespace ITk
{

//----------------------------------------------------------------------
// Constructor with parameters:
//----------------------------------------------------------------------
StripDigitization::StripDigitization(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator)
{
}

//----------------------------------------------------------------------
// Initialize method:
//----------------------------------------------------------------------
StatusCode StripDigitization::initialize() {
  ATH_MSG_DEBUG("StripDigitization::initialize()");
  return StatusCode::SUCCESS ;
}

//----------------------------------------------------------------------
// Execute method:
//----------------------------------------------------------------------

StatusCode StripDigitization::execute(const EventContext& ctx) {
  ATH_MSG_DEBUG("execute()");
  return m_stripDigitizationTool->processAllSubEvents(ctx);
}

} // namespace ITk
