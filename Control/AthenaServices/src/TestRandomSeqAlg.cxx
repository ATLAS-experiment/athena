/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/IAtRndmGenSvc.h"
#include "CLHEP/Random/RandomEngine.h"
#include "TestRandomSeqAlg.h"


StatusCode TestRandomSeqAlg::initialize() {
  ATH_MSG_INFO("Initializing");
  ATH_CHECK( m_rndmSvc.retrieve() );
  ATH_CHECK( (m_pEng = m_rndmSvc->GetEngine(m_streamName)) != nullptr );
  return StatusCode::SUCCESS;
}


StatusCode TestRandomSeqAlg::execute() {
  msg() << MSG::DEBUG << "execute: random sequence: ";
  for (int i=0; i<m_noOfNo.value(); ++i) msg() << m_pEng->flat() << " ";
  msg() << endmsg;
  return StatusCode::SUCCESS;
}
