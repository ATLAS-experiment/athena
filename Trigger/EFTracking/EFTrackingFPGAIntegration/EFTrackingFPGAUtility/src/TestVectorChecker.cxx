/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "EFTrackingFPGAUtility/TestVectorChecker.h"

namespace EFTrackingFPGAUtility{
TestVectorChecker::TestVectorChecker(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode TestVectorChecker::initialize() {
  ATH_MSG_INFO("Initializing " << name());
  ATH_CHECK(m_outputDataStreamAKey.initialize());
  ATH_CHECK(m_outputDataStreamBKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode TestVectorChecker::execute(const EventContext& ctx) const {
  SG::ReadHandle<std::vector<unsigned long>> outputDataStreamA(
    m_outputDataStreamAKey,
    ctx
  );

  SG::ReadHandle<std::vector<unsigned long>> outputDataStreamB(
    m_outputDataStreamAKey,
    ctx
  );

  ATH_CHECK(outputDataStreamA->size() == outputDataStreamB->size());

  for (std::size_t index = 0; index < outputDataStreamA->size(); index++) {
    ATH_CHECK(outputDataStreamA->at(index) == outputDataStreamB->at(index));
  }
  
  return StatusCode::SUCCESS;
}
}

