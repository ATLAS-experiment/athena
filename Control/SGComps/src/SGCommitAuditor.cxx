/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SGCommitAuditor.h"

#include "AthenaBaseComps/AthCheckMacros.h"


SGCommitAuditor::SGCommitAuditor(const std::string& name,
				 ISvcLocator* pSvcLocator) 
  :Auditor(name,pSvcLocator),
   p_sg("StoreGateSvc", name)
{

}

SGCommitAuditor::~SGCommitAuditor() {
}


StatusCode
SGCommitAuditor::initialize() {
  ATH_CHECK( p_sg.retrieve() );

  return StatusCode::SUCCESS;
}


void
SGCommitAuditor::after(const std::string& event, const std::string& /*name*/,
                       const EventContext&, const StatusCode&) {
  if (event != Gaudi::IAuditor::Execute) {
    return;
  }
  p_sg->commitNewDataObjects();
}

