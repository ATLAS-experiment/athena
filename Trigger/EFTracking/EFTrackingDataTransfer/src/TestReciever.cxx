/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TestReciever.h"

TestReciever::TestReciever(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

TestReciever::~TestReciever()
{
}

StatusCode TestReciever::initialize()
{
  ATH_CHECK(m_inputKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode TestReciever::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode TestReciever::execute(const EventContext& context) const
{
  auto handle = SG::makeHandle(m_inputKey, context);
  ATH_MSG_DEBUG("Waiting for response");
  handle->waitForResponse();
  ATH_MSG_DEBUG("Ready");

  ATH_MSG_DEBUG("Response id " << handle->response()->id() << " , event number " << handle->response()->eventinfo().eventnumber());
  return StatusCode::SUCCESS;
}

