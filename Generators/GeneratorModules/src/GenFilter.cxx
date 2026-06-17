/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorModules/GenFilter.h"


GenFilter::GenFilter(const std::string& name, ISvcLocator* pSvcLocator)
  : GenBase(name, pSvcLocator)
{
  declareProperty("TotalPassed", m_nNeeded=-1);
#ifdef HEPMC3
  declareProperty("KeepAllEvents", m_keepAll=false);
#endif
  m_nPass = 0;
  m_nFail = 0;
}


StatusCode GenFilter::initialize() {
  CHECK(GenBase::initialize());
  m_nPass = 0;
  m_nFail = 0;
  CHECK(filterInitialize());
  return StatusCode::SUCCESS;
}


StatusCode GenFilter::execute(const EventContext& ctx) {
  if (events_const()->empty()) {
    ATH_MSG_ERROR("No events found in McEventCollection");
    return StatusCode::FAILURE;
  } else if (events_const()->size() > 1) {
    /// @todo Probably the filter should only look at the first event... right?
    ATH_MSG_WARNING("More than one event in current McEventCollection -- which is valid?");
  }
  StatusCode sc = filterEvent(ctx);
#ifdef HEPMC3
  if (filterPassed(ctx) || m_keepAll ) {
#else
  if (filterPassed(ctx) ) {
#endif
    ATH_MSG_DEBUG("Event passed filter");
    m_nPass += 1;
  } else {
    ATH_MSG_DEBUG("Event failed filter");
    m_nFail += 1;
  }
  // Bail out once we have enough events
  if (m_nPass >= m_nNeeded && m_nNeeded > 0)
    sc = StatusCode::FAILURE;
  return sc;
}


StatusCode GenFilter::finalize() {
  ATH_MSG_INFO("Events passed = " << m_nPass << "  Events failed = " << m_nFail);
  CHECK(filterFinalize());
  return StatusCode::SUCCESS;
}
