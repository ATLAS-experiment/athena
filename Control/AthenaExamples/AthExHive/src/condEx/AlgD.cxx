/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AlgD.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandle.h"

//---------------------------------------------------------------------------

StatusCode AlgD::initialize() {
  ATH_MSG_DEBUG("initialize {}", name());

  ATH_CHECK( m_rdh1.initialize() );
  ATH_CHECK( m_rch1.initialize() );
  ATH_CHECK( m_rch2.initialize() );

  ATH_MSG_INFO( "m_rdh1 id: {}", m_rdh1.key() );
  ATH_MSG_INFO( "m_rch1 id: {}", m_rch1.key() );
  ATH_MSG_INFO( "m_rch2 id: {}", m_rch2.key() );

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgD::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("execute {}", name());

  SG::ReadHandle<HiveDataObj> rh1(m_rdh1, ctx);
  if (!rh1.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key {}", m_rdh1.key());
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("  read: {} = {}", rh1.key(), rh1->val() );

  SG::ReadCondHandle<CondDataObj> ch1(m_rch1, ctx);
  SG::ReadCondHandle<CondDataObj> ch2(m_rch2, ctx);

  const CondDataObj *cdo = *ch1;
  if (cdo != 0) {
    ATH_MSG_INFO("  CDO1: {}", *cdo );
  } else {
    ATH_MSG_ERROR("  CDO1 ptr == zero");
  }

  cdo = *ch2;
  if (cdo != 0) {
    ATH_MSG_INFO("  CDO2: {}", *cdo );
  } else {
    ATH_MSG_ERROR("  CDO2 ptr == zero");
  }

  ATH_MSG_INFO("test: {} {} {} {}",
               ctx.eventID().event_number(), rh1->val(), **ch1, **ch2);
  
  return StatusCode::SUCCESS;

}

