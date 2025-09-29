/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


#include <EventBookkeeperTools/FilterReporter.h>
#include <xAODCutFlow/CutBookkeeper.h>

#include "AllWrittenEventsCounterAlg.h"


AllWrittenEventsCounterAlg::AllWrittenEventsCounterAlg(const std::string& name,
                                                       ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}


StatusCode AllWrittenEventsCounterAlg::initialize ATLAS_NOT_THREAD_SAFE ()
{
  ATH_MSG_DEBUG("Initializing " << this->name() << "...");

  ATH_CHECK(m_filterParams.initialize());

  m_filterParams.cutFlowSvc()->registerTopFilter(m_filterParams.key(),
                                                 m_filterParams.description(),
                                                 xAOD::CutBookkeeper::CutLogic::ALLEVENTSWRITTEN,
                                                 "AllStreams",
                                                 false);

  return StatusCode::SUCCESS;
}


StatusCode AllWrittenEventsCounterAlg::execute(const EventContext& ctx) const
{
  FilterReporter filter(m_filterParams, true, ctx);

  return StatusCode::SUCCESS;
}


StatusCode AllWrittenEventsCounterAlg::finalize()
{
  ATH_MSG_DEBUG("Finalizing " << this->name() << "...");

  ATH_MSG_INFO(m_filterParams.summary());

  return StatusCode::SUCCESS;
}
