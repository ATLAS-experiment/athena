/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AsyncMPIComputeAlg.h"

AsyncMPIomputeAlg::AsyncMPIomputeAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

AsyncMPIomputeAlg::~AsyncMPIomputeAlg()
{
}

StatusCode AsyncMPIomputeAlg::initialize()
{
  //ATH_MSG_DEBUG("Use macros for logging!");
  return StatusCode::SUCCESS;
}

StatusCode AsyncMPIomputeAlg::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode AsyncMPIomputeAlg::execute(const EventContext& context) const
{
  return StatusCode::SUCCESS;
}

