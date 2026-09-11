/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArEventTest/FakeLArTimeOffset.h"
#include "LArRawConditions/LArFEBTimeOffset.h"
#include "LArRawConditions/LArGlobalTimeOffset.h"

FakeLArTimeOffset::FakeLArTimeOffset(const std::string & name, ISvcLocator * pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{
  declareProperty("FEBids",m_FEBids);
  declareProperty("FEbTimeOffsets",m_FEBtimeOffsets);
  declareProperty("GlobalTimeOffset",m_globalTimeOffset=0);
  declareProperty("FebTimeOffsetKey",m_keyFebOffset="FebTimeOffset");
  declareProperty("GlobalTimeOffsetKey",m_keyGlobalOffset="GlobalTimeOffset");
}

FakeLArTimeOffset::~FakeLArTimeOffset() {}

StatusCode FakeLArTimeOffset::initialize() {
  ATH_MSG_DEBUG ( "Initialize..." );
  //Check Consistency
  if (m_FEBtimeOffsets.size()!=m_FEBids.size()) {
    ATH_MSG_ERROR ( "Problem with jobOpts: Have " << m_FEBtimeOffsets.size() << " time offset values for " 
                    << m_FEBids.size() << " FEBs." );
    return StatusCode::FAILURE;
  }
  LArGlobalTimeOffset* globalTimeOffset=new LArGlobalTimeOffset();
  globalTimeOffset->setTimeOffset(m_globalTimeOffset);
  ATH_CHECK( detStore()->record(globalTimeOffset,m_keyGlobalOffset) );

  LArFEBTimeOffset* febTimeOffset=new LArFEBTimeOffset();
  for (unsigned i=0;i<m_FEBids.size();i++) {
    const HWIdentifier id(m_FEBids[i]);
    febTimeOffset->setTimeOffset(id,m_FEBtimeOffsets[i]);
    ATH_MSG_DEBUG ( "FEB 0x" << MSG::hex << id << " Offset=" << m_FEBtimeOffsets[i] );
  }
  ATH_CHECK( detStore()->record(febTimeOffset,m_keyFebOffset) );
  return StatusCode::SUCCESS;
}



StatusCode FakeLArTimeOffset::execute(const EventContext& /*ctx*/)
{return StatusCode::SUCCESS;}

StatusCode FakeLArTimeOffset::finalize()
{return StatusCode::SUCCESS;}
