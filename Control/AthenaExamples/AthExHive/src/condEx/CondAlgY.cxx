/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CondAlgY.h"
#include "AthExHive/IASCIICondDbSvc.h"
#include "StoreGate/WriteCondHandle.h"


StatusCode CondAlgY::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  ATH_CHECK( m_cds.retrieve() );

  m_wch1.setDbKey(m_dbk1);
  m_wch2.setDbKey(m_dbk2);

  ATH_CHECK( m_wch1.initialize() );
  ATH_CHECK( m_wch2.initialize() );

  return StatusCode::SUCCESS;
}

StatusCode CondAlgY::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("execute " << name());
  EventIDBase now(ctx.eventID());

  SG::WriteCondHandle<CondDataObjY> wch1(m_wch1,ctx);
  SG::WriteCondHandle<CondDataObjY> wch2(m_wch2,ctx);

  // do we have a valid m_wch for current time?
  if ( wch1.isValid(now) ) {
    ATH_MSG_DEBUG(" Found a valid write handle for " << wch1.key());
  }
  else {

    ATH_MSG_DEBUG("  CondHandle " << wch1.key() 
                  << " not valid. Getting new info for dbKey \"" 
                  << wch1.dbKey() << "\" from CondDb");

    EventIDRange r;
    IASCIICondDbSvc::dbData_t val;
    if (m_cds->getRange(wch1.dbKey(), ctx, r, val).isFailure()) {
      ATH_MSG_ERROR("  could not find dbKey \"" << wch1.dbKey() 
		    << "\" in CondSvc registry");
      return StatusCode::FAILURE;
    }

    CondDataObjY* cdo = new CondDataObjY( val );
    if (wch1.record(r, cdo).isFailure()) {
      ATH_MSG_ERROR("could not record CondDataObjY " << wch1.key() 
		    << " = " << *cdo
                    << " with EventRange " << r);
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("recorded new CDO " << wch1.key() << " = " << *cdo 
		 << " with range " << r);
  }

  // do we have a valid wch for current time?
  if ( wch2.isValid(now) ) {
    ATH_MSG_DEBUG(" Found a valid write handle for " << wch2.key());
  }
  else {

    ATH_MSG_DEBUG("  CondHandle " << wch2.key() 
                  << " not valid. Getting new info for dbKey \"" 
                  << wch2.dbKey() << "\" from CondDb");

    EventIDRange r;
    IASCIICondDbSvc::dbData_t val;
    if (m_cds->getRange(wch2.dbKey(), ctx, r, val).isFailure()) {
      ATH_MSG_ERROR("  could not find dbKey \"" << wch2.dbKey() 
		    << "\" in CondSvc registry");
      return StatusCode::FAILURE;
    }

    CondDataObjY* cdo = new CondDataObjY( val );
    if (wch2.record(r, cdo).isFailure()) {
      ATH_MSG_ERROR("could not record CondDataObjY " << wch2.key() 
		    << " = " << *cdo
                    << " with EventRange " << r);
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("recorded new CDO " << wch2.key() << " = " << *cdo 
		 << " with range " << r);
  }

  return StatusCode::SUCCESS;

}

