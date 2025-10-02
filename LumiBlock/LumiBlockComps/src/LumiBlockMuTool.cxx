/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LumiBlockMuTool.h"
#include "StoreGate/ReadDecorHandle.h"

//--------------------------------------------------

StatusCode
LumiBlockMuTool::initialize()
{
  ATH_MSG_DEBUG("LumiBlockMuTool::initialize() begin");
  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_rdhkActMu.initialize());
  ATH_CHECK(m_rdhkAveMu.initialize());
  return StatusCode::SUCCESS;
}

float
LumiBlockMuTool::actualInteractionsPerCrossing(const EventContext& ctx) const {

  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
  SG::ReadDecorHandle<xAOD::EventInfo,float> actMu(m_rdhkActMu, ctx);
  float mu = actMu.isPresent() ? actMu(0) : 0.;
  return mu;
}

float
LumiBlockMuTool::averageInteractionsPerCrossing(const EventContext& ctx) const{

  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
  SG::ReadDecorHandle<xAOD::EventInfo,float> aveMu(m_rdhkAveMu, ctx);
  float mu = aveMu.isPresent() ? aveMu(0) : 0.;
  return mu;
}
