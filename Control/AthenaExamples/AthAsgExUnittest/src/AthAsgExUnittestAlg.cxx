
//
//  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//

// AthAsgExUnittest includes
#include "AthAsgExUnittestAlg.h"

AthAsgExUnittestAlg::AthAsgExUnittestAlg( const std::string& name, 
			    ISvcLocator* pSvcLocator ) : 
  AthAnalysisAlgorithm( name, pSvcLocator ) {
  addRef(); // workaround until fix in Gaudi
}


AthAsgExUnittestAlg::~AthAsgExUnittestAlg() {}


StatusCode AthAsgExUnittestAlg::initialize() {
  ATH_MSG_INFO( "Initializing " << name() << "..." );
  ATH_MSG_INFO( "MyProperty = " << m_property );
  CHECK(m_tool.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode AthAsgExUnittestAlg::finalize() {
  ATH_MSG_INFO( "Finalizing " << name() << "..." );
  return StatusCode::SUCCESS;
}

StatusCode AthAsgExUnittestAlg::execute() {  
  ATH_MSG_DEBUG( "Executing " << name() << "..." );
  setFilterPassed(false); //optional: start with algorithm not passed

  // Real algorithm here

  setFilterPassed(true); //if got here, assume that means algorithm passed
  return StatusCode::SUCCESS;
}

