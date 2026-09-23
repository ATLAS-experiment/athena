/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/cPtrAccessSEGVHandler.h"

#include "PageAccessControlSvc.h"
PageAccessControlSvc::PageAccessControlSvc( const std::string& name, 
					    ISvcLocator* pSvcLocator ) : 
  base_class(name, pSvcLocator), m_saveSEGVaction(),
  m_accessControl(), m_SEGVHandler(m_accessControl)
{
  //pass m_SEGVHandler pointer to cPtrAccessSEGVHandler
  setPtrAccessSEGVHandler(&m_SEGVHandler);
  declareProperty("AutoMonitoring", m_autoMonitor=true, 
		  "start monitoring on initialize, stop on finalize");
}

bool PageAccessControlSvc::startMonitoring () {
  int rc = sigaction(SIGSEGV,nullptr, &m_saveSEGVaction);
  if (0==rc) {
    struct sigaction sa(m_saveSEGVaction);
    sa.sa_sigaction= cPtrAccessSEGVHandler; 
    sa.sa_flags=SA_SIGINFO;
    //off we go
    rc=sigaction(SIGSEGV,&sa,nullptr);
  }
  return (0==rc);
}

bool PageAccessControlSvc::stopMonitoring () {
  return (0 == sigaction(SIGSEGV,&m_saveSEGVaction, nullptr));
}

/// has this pointer been accessed (read/written)
bool PageAccessControlSvc::accessed(const void* address) const {
  return m_accessControl.accessed(address);
}

StatusCode PageAccessControlSvc::initialize() {
  StatusCode sc(StatusCode::SUCCESS);
  ATH_MSG_INFO ("Initializing {}", name());
  if (m_autoMonitor.value() && !this->startMonitoring()) sc = StatusCode::FAILURE;
  return sc;
}

StatusCode PageAccessControlSvc::finalize() {
  StatusCode sc(StatusCode::SUCCESS);
  if (m_autoMonitor.value()) {
    if (this->stopMonitoring()) this->report();
    else sc = StatusCode::FAILURE;
  }
  return sc;
}

void PageAccessControlSvc::report() const {
  ATH_MSG_INFO( "Access monitoring report" );
  PtrAccessSEGVHandler::const_iterator i(m_SEGVHandler.beginAccessedPtrs()),
    e(m_SEGVHandler.endAccessedPtrs());
  while (i != e) {
    ATH_MSG_DEBUG( "accessed pointer at @{}", *i++ );
  }
  PageAccessControl::const_iterator ia(m_accessControl.beginProtectedPtrs()),
    ea(m_accessControl.endProtectedPtrs());
  while (ia != ea) {
    ATH_MSG_DEBUG( "protected page at @{} accessed {} times",
                   ia->addr, ia->restored);
    ++ia;
  }
}
