/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigCostMonitor/TrigCostAuditor.h"
#include "TrigCostSvc.h"


/////////////////////////////////////////////////////////////////////////////

TrigCostAuditor::TrigCostAuditor(const std::string& name, ISvcLocator* pSvcLocator) :
Auditor(name, pSvcLocator),
AthMessaging(msgSvc(), name)
{
  ATH_MSG_DEBUG("TrigCostAuditor constructor");
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode TrigCostAuditor::initialize(){
  ATH_MSG_DEBUG("TrigCostAuditor initialize()");
  ATH_CHECK( m_trigCostSvcHandle.retrieve() );
  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode TrigCostAuditor::finalize() {
  ATH_MSG_DEBUG("TrigCostAuditor finalize()");
  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void TrigCostAuditor::before(const std::string& event, const std::string& caller,
                             const EventContext& ctx) {
  if (event != IAuditor::Execute) return; // I only care for execution time
  ATH_MSG_DEBUG("Before Execute: " << caller);
  callService(caller, ITrigCostSvc::AuditType::Before, ctx);

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void TrigCostAuditor::after(const std::string& event, const std::string& caller,
                            const EventContext& ctx, const StatusCode& sc) {
  if (event != IAuditor::Execute) return; // I only care for execution time
  ATH_MSG_DEBUG("After Execute: " << caller << " " << sc);
  callService(caller, ITrigCostSvc::AuditType::After, ctx);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void TrigCostAuditor::callService(const std::string& caller, ITrigCostSvc::AuditType type,
                                  const EventContext& ctx) {
  if (m_trigCostSvcHandle->processAlg(ctx, caller, type).isFailure()) {
    ATH_MSG_FATAL("Error in TrigCostSvc called by TrigCostAuditor, auditing algorithm: " << caller);
    throw std::runtime_error("TrigCostAuditor exception");
  }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


