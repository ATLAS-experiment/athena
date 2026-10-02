/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CondAlgX.h"
#include "AthExHive/IASCIICondDbSvc.h"

#include "StoreGate/WriteCondHandle.h"

#include "GaudiKernel/ServiceHandle.h"

#include "GaudiKernel/EventIDBase.h"
#include "GaudiKernel/EventIDRange.h"

#include <chrono>


StatusCode CondAlgX::initialize() {
  ATH_MSG_DEBUG("initialize {}", name());

  ATH_CHECK( m_evt.initialize() );

  ATH_CHECK( m_cds.retrieve() );

  m_wchk.setDbKey(m_dbKey);
  ATH_CHECK( m_wchk.initialize() );

  return StatusCode::SUCCESS;
}

StatusCode CondAlgX::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("execute {}", name());
  
  SG::ReadHandle<xAOD::EventInfo> evt( m_evt, ctx );
  if (!evt.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve EventInfo");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("   EventInfo:  r: {} e: {}",
                evt->runNumber(), evt->eventNumber() );

  EventIDBase now(ctx.eventID());
  if (evt->eventNumber() == 10) {
      std::this_thread::sleep_for(std::chrono::milliseconds( 500 ));
  }

  SG::WriteCondHandle<CondDataObj> wch(m_wchk,ctx);
  // do we have a valid m_wch for current time?
  if ( wch.isValid(now) ) {
    ATH_MSG_DEBUG("CondHandle is already valid for " << now
		  << ". In theory this should not be called, but may happen"
		  << " if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;

  }

  ATH_MSG_DEBUG("  CondHandle {} not valid. Getting new info for dbKey \"{}\" from CondDb",
                wch.key(), wch.dbKey());

  EventIDRange r;
  IASCIICondDbSvc::dbData_t val;
  if (m_cds->getRange(wch.dbKey(), ctx, r, val).isFailure()) {
    ATH_MSG_ERROR("  could not find dbKey \"{}\" in CondSvc registry",
                  wch.dbKey() );
    return StatusCode::FAILURE;
  }

  CondDataObj* cdo = new CondDataObj( val );
  if (wch.record(r, cdo).isFailure()) {
    ATH_MSG_ERROR("could not record CondDataObj {} = {} with EventRange {}",
                  wch.key(), *cdo, static_cast<std::string>(r));
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("recorded new CDO {} = {} with range {}",
               wch.key(), *cdo, static_cast<std::string>(r));
  
  return StatusCode::SUCCESS;
}

