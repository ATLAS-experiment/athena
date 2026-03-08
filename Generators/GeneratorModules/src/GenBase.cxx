/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeneratorModules/GenBase.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/DataSvc.h"
#include <fstream>


GenBase::GenBase(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{
}


StatusCode GenBase::initialize() {
  ATH_CHECK( m_mcevents_const.initialize() );
  m_mcEventKey = m_mcevents_const.key();

  // Get the particle property service
  ATH_CHECK(m_ppSvc.retrieve());
  return StatusCode::SUCCESS;
}


/// Access the current event's McEventCollection
McEventCollection* GenBase::events ATLAS_NOT_CONST_THREAD_SAFE () {
  // Make a new MC event collection if necessary
  McEventCollection* mcevents = nullptr;
  if (!evtStore()->contains<McEventCollection>(m_mcEventKey) && m_mkMcEvent) {
    ATH_MSG_DEBUG("Creating new McEventCollection in the event store");
    mcevents = new McEventCollection();
    if (evtStore()->record(mcevents, m_mcEventKey).isFailure())
      ATH_MSG_ERROR("Failed to record a new McEventCollection");
  }
  else {
    const McEventCollection* mecc = 0;

    if (evtStore()->retrieve (mecc, m_mcEventKey).isFailure()) 
      ATH_MSG_ERROR("Failed to retrieve McEventCollection");

    mcevents = const_cast<McEventCollection*> (&(*mecc));
  }

  return mcevents;
}

