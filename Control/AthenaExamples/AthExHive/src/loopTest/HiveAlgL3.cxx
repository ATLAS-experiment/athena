/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HiveAlgL3.h"
#include "GaudiKernel/ServiceHandle.h"
#include <thread>
#include <chrono>
#include <memory>

HiveAlgL3::HiveAlgL3( const std::string& name, 
                      ISvcLocator* pSvcLocator ) : 
  ::HiveAlgBase( name, pSvcLocator )
{
}

HiveAlgL3::~HiveAlgL3() = default;

StatusCode HiveAlgL3::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  ATH_CHECK( m_udh1.initialize() );

  return HiveAlgBase::initialize();
}

StatusCode HiveAlgL3::execute(const EventContext& ctx) const{

  ATH_MSG_DEBUG("execute " << name());

  sleep(ctx);

  SG::UpdateHandle<HiveDataObj> udh1{m_udh1, ctx};

  udh1->val( udh1->val() + 1);

  ATH_MSG_INFO("  update: " << udh1.key() << " = " << udh1->val() );

  return StatusCode::SUCCESS;

}

