/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HiveAlgF.h"

HiveAlgF::HiveAlgF( const std::string& name, 
		    ISvcLocator* pSvcLocator ) : 
  ::HiveAlgBase( name, pSvcLocator )
{
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
HiveAlgF::~HiveAlgF() = default;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode HiveAlgF::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  ATH_CHECK( m_rdh1.initialize() );
  ATH_CHECK( m_rdh2.initialize() );
  ATH_CHECK( m_rdh3.initialize() );
  ATH_CHECK( m_rdh4.initialize() );
  ATH_CHECK( m_rdh5.initialize() );
  ATH_CHECK( m_rdh6.initialize() );

  // initialize base class
  return HiveAlgBase::initialize ();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode HiveAlgF::execute(const EventContext& ctx) const {

  ATH_MSG_DEBUG("execute " << name());
 
  sleep(ctx);

  SG::ReadHandle<HiveDataObj> rdh1{m_rdh1, ctx};
  if (!rdh1.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << rdh1.key());
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<HiveDataObj> rdh2{m_rdh2, ctx};
  if (!rdh2.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << rdh2.key());
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<HiveDataObj> rdh3{m_rdh3, ctx};
  if (!rdh3.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << rdh3.key());
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<HiveDataObj> rdh4{m_rdh4, ctx};
  if (!rdh4.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << rdh4.key());
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<HiveDataObj> rdh5{m_rdh5, ctx};
  if (!rdh5.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << rdh5.key());
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<HiveDataObj> rdh6{m_rdh6, ctx};
  if (!rdh6.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << rdh6.key());
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("  read: " << rdh1.key() << " = " << rdh1->val() );
  ATH_MSG_INFO("  read: " << rdh2.key() << " = " << rdh2->val() );
  ATH_MSG_INFO("  read: " << rdh3.key() << " = " << rdh3->val() );

  ATH_MSG_INFO("test: " << ctx.eventID().event_number() << " "
               << rdh1->val() + rdh2->val() + rdh3->val() 
               << " " << rdh1->val()
               << " " << rdh2->val()
               << " " << rdh3->val()
               << " " << rdh4->val()
               << " " << rdh5->val()
               << " " << rdh6->val()
               );

  return StatusCode::SUCCESS;

}

