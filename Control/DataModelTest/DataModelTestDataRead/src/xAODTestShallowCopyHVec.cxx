/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataRead/src/xAODTestShallowCopyHVec.cxx
 * @author snyder@bnl.gov
 * @date Nov 2020, from earlier version Jun, 2014
 * @brief Algorithm to test shallow-copy of xAOD data.
 */


#include "xAODTestShallowCopyHVec.h"
#include "DataModelTestDataRead/HVec.h"
#include "DataModelTestDataRead/H.h"
#include "DataModelTestDataRead/HAuxContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthenaKernel/errorcheck.h"


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestShallowCopyHVec::initialize()
{
  ATH_CHECK( m_readKey.initialize() );
  ATH_CHECK( m_writeKey.initialize() );
  ATH_CHECK( m_aFloat20Key.initialize() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestShallowCopyHVec::execute (const EventContext& ctx) const
{
  size_t count = ctx.evt() + 1;

  {
    SG::ReadHandle<DMTest::HVec> vec (m_readKey, ctx);
    auto ret = xAOD::shallowCopy (*vec, ctx);

    SG::WriteHandle<DMTest::HVec> copy (m_writeKey, ctx);
    ATH_CHECK( copy.record (std::move(ret.first), std::move(ret.second)) );

    SG::WriteDecorHandle<DMTest::HVec, float> aFloat20 (m_aFloat20Key, ctx);
    for (H* h : *copy)
      aFloat20(*h) = count * 20000 + h->aFloat();
  }

  return StatusCode::SUCCESS;
}


} // namespace DMTest
