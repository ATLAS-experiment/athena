/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArG4ShowerLibSvcTest.h"

LArG4ShowerLibSvcTest::LArG4ShowerLibSvcTest (const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{
}

StatusCode LArG4ShowerLibSvcTest::initialize()
{
  ATH_MSG_INFO("Initializing");
  ATH_CHECK(m_showerLibSvc.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode LArG4ShowerLibSvcTest::finalize()
{
  ATH_MSG_INFO("Finalized");
  return StatusCode::SUCCESS;
}

StatusCode LArG4ShowerLibSvcTest::execute()
{
  ATH_MSG_INFO("execute: stub");
  return StatusCode::SUCCESS;
}
