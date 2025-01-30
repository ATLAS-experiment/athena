/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AlgC.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandle.h"

AlgC::AlgC( const std::string& name, 
            ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator )
{}

//---------------------------------------------------------------------------

StatusCode AlgC::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  ATH_CHECK(m_rdh1.initialize());
  ATH_CHECK(m_rch.initialize());

  ATH_MSG_INFO( "m_rdh1 id: " << m_rdh1.fullKey() );
  ATH_MSG_INFO( "m_rch id:  " << m_rch.fullKey() );


  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgC::execute() {
  ATH_MSG_DEBUG("execute " << name());

  SG::ReadHandle<HiveDataObj> rh1(m_rdh1);
  if (!rh1.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << m_rdh1.key());
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("  read: " << rh1.key() << " = " << rh1->val() );

  EventIDBase t( getContext().eventID() );
  
  SG::ReadCondHandle<CondDataObj> rch( m_rch );
  const CondDataObj *cdo = *rch;
  if (cdo != 0) {
    ATH_MSG_INFO("  read CH: " << rch.key() << " = " << *cdo );
  } else {
    ATH_MSG_ERROR("  CDO ptr for " << rch.key() << " == zero");
  }
  
  return StatusCode::SUCCESS;

}

