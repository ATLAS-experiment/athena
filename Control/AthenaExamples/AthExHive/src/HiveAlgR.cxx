/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HiveAlgR.h"
#include "GaudiKernel/ServiceHandle.h"

HiveAlgR::HiveAlgR( const std::string& name, 
		    ISvcLocator* pSvcLocator ) : 
  ::AthReentrantAlgorithm( name, pSvcLocator )
{}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
HiveAlgR::~HiveAlgR() = default;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode HiveAlgR::initialize() {

  ATH_MSG_INFO( "initialize: {}", index() );

  ATH_CHECK( m_wrh1.initialize() );
  ATH_CHECK( m_evt.initialize() );

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode HiveAlgR::finalize() {
  ATH_MSG_INFO( "finalize: {}", index() );
  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode HiveAlgR::execute(const EventContext& ctx) const {

  ATH_MSG_INFO( "execute: {} on {}", index(), ctx );

  SG::ReadHandle<xAOD::EventInfo> evt(m_evt,ctx);
  ATH_MSG_INFO("   EventInfo:  r: {} e: {}",
               evt->runNumber(), evt->eventNumber() );

  SG::WriteHandle<HiveDataObj> wh1(m_wrh1,ctx);
  ATH_CHECK(wh1.record(std::make_unique<HiveDataObj>(10000 +evt->eventNumber()*100)));

  ATH_MSG_INFO("  write: {} = {}", wh1.key(), wh1->val() );

  return StatusCode::SUCCESS;

}

