/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AlgT.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

AlgT::AlgT( const std::string& name, 
	    ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator )
{}

//---------------------------------------------------------------------------

StatusCode AlgT::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  if (m_rdh1.key() != "") ATH_CHECK( m_rdh1.initialize() );
  ATH_CHECK( m_wrh1.initialize() );
  ATH_CHECK( m_evt.initialize() );

  ATH_CHECK( m_tool1.retrieve() );
  ATH_CHECK( m_tool2.retrieve() );
  ATH_CHECK( m_tool3.retrieve() );

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgT::execute() {

  ATH_MSG_DEBUG("execute " << name());

  SG::ReadHandle<xAOD::EventInfo> evt(m_evt);
  ATH_MSG_INFO("   EventInfo:  r: " << evt->runNumber()
               << " e: " << evt->eventNumber() );


  SG::WriteHandle<HiveDataObj> wh1(m_wrh1);
  ATH_CHECK( wh1.record( std::make_unique<HiveDataObj> (10000 + evt->eventNumber())));

  ATH_MSG_INFO("  write: " << wh1.key() << " = " << wh1->val() );


  ATH_CHECK(m_tool1->doSomething());
  ATH_CHECK(m_tool2->doSomething());
  ATH_CHECK(m_tool3->doSomething());
  

  return StatusCode::SUCCESS;

}

