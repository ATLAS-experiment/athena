/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HiveAlgL2.h"
#include "GaudiKernel/ServiceHandle.h"
#include <thread>
#include <chrono>
#include <memory>

HiveAlgL2::HiveAlgL2( const std::string& name, 
                      ISvcLocator* pSvcLocator ) : 
  ::HiveAlgBase( name, pSvcLocator )
{
}

HiveAlgL2::~HiveAlgL2() = default;

StatusCode HiveAlgL2::initialize() {
  ATH_MSG_DEBUG("initialize {}", name());

  ATH_CHECK( m_rdh1.initialize() );
  ATH_CHECK( m_udh1.initialize() );

  return HiveAlgBase::initialize();
}

StatusCode HiveAlgL2::execute(const EventContext& ctx) const {

  ATH_MSG_DEBUG("execute {}", name());

  sleep(ctx);

  SG::ReadHandle<HiveDataObj> rdh1{m_rdh1, ctx};
  if (!rdh1.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key {}", rdh1.key());
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("  read: {} = {}", rdh1.key(), rdh1->val() );
  
  SG::UpdateHandle<HiveDataObj> udh1{m_udh1, ctx};

  udh1->val( udh1->val() + 1);

  ATH_MSG_INFO("  update: {} = {}", udh1.key(), udh1->val() );

  return StatusCode::SUCCESS;

}

