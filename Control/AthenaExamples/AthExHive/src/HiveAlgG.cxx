/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HiveAlgG.h"

HiveAlgG::HiveAlgG( const std::string& name, 
		    ISvcLocator* pSvcLocator ) : 
  ::HiveAlgBase( name, pSvcLocator )
{
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
HiveAlgG::~HiveAlgG() = default;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode HiveAlgG::initialize() {
  ATH_MSG_DEBUG("initialize {}", name());

  ATH_CHECK( m_rdh1.initialize() );
  ATH_CHECK( m_wrh1.initialize() );

  // initialize base class
  return HiveAlgBase::initialize();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode HiveAlgG::execute(const EventContext& ctx) const {

  ATH_MSG_DEBUG("execute {}", name());

  sleep(ctx);

  SG::ReadHandle<HiveDataObj> rdh1{m_rdh1, ctx};
  if (!rdh1.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key {}", rdh1.key());
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("  read: {} = {}", rdh1.key(), rdh1->val() );
  
  SG::WriteHandle<HiveDataObj> wrh1{m_wrh1, ctx};
  ATH_CHECK(wrh1.record(std::make_unique< HiveDataObj >(70000)));

  ATH_MSG_INFO("  write: {} = {}", wrh1.key(), wrh1->val() );

  return StatusCode::SUCCESS;

}

