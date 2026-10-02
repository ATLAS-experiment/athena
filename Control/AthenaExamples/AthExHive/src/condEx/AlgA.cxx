/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AlgA.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <memory>

//---------------------------------------------------------------------------

StatusCode AlgA::initialize() {
  ATH_MSG_DEBUG("initialize {}", name());

  ATH_CHECK( m_wrh1.initialize() );
  ATH_CHECK( m_wrh2.initialize() );
  ATH_CHECK( m_evt.initialize() );

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgA::execute(const EventContext& ctx) const {

  ATH_MSG_DEBUG("execute {}", name());

  SG::ReadHandle<xAOD::EventInfo> evt(m_evt, ctx);
  ATH_MSG_INFO("   EventInfo:  r: {} e: {} evt: {}",
               evt->runNumber(), evt->eventNumber(), ctx.evt() );


  const unsigned int i = ctx.evt() + 1;

  SG::WriteHandle<HiveDataObj> wh1(m_wrh1, ctx);
  ATH_CHECK( wh1.record( std::make_unique<HiveDataObj> 
                         ( 10000 + 
			   evt->eventNumber()*100 + 
			   i)  )
             );
  ATH_MSG_INFO("  write: {} = {}", wh1.key(), wh1->val() );


  SG::WriteHandle<HiveDataObj> wh2(m_wrh2, ctx);
  ATH_CHECK( wh2.record( std::make_unique< HiveDataObj >( 10050+i ) ) );
  ATH_MSG_INFO("  write: {} = {}", wh2.key(), wh2->val() );
    
  return StatusCode::SUCCESS;

}

