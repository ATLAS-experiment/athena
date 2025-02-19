/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthIncFirerAlg.h"
#include "GaudiKernel/Incident.h"


AthIncFirerAlg::AthIncFirerAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator),
  m_incSvc("IncidentSvc", name)
{}


StatusCode AthIncFirerAlg::initialize() {
  ATH_CHECK( m_incSvc.retrieve() );

  if (m_incidents.empty()) {
    ATH_MSG_ERROR("Need to have at least one incident defined!" <<name());
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}


StatusCode AthIncFirerAlg::execute(const EventContext& ctx) const {

  for (const std::string& inc : m_incidents) {
    ATH_MSG_VERBOSE("Firing incident " << inc);
    if (m_serial) {
      m_incSvc->fireIncident(Incident(name(), inc, ctx));
    }
    else {
      m_incSvc->fireIncident(std::make_unique<Incident>(name(), inc, ctx));
    }
  }
  return StatusCode::SUCCESS;
}


