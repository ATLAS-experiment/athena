/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestThinJVec.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief Thin JVecContainer objects.
 */


#include "xAODTestThinJVec.h"
#include "StoreGate/ThinningHandle.h"


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestThinJVec::initialize()
{
  ATH_CHECK( m_jvecContainerKey.initialize (m_stream) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestThinJVec::execute (const EventContext& ctx) const
{
  SG::ThinningHandle<DMTest::JVecContainer> jvec(m_jvecContainerKey, ctx);

  unsigned int evt = ctx.evt();

  jvec.keepAll();
  for (size_t i = 0; i < jvec->size(); i++) {
    if (((evt ^ m_mask) & (1 << (i%4))) == 0) {
      jvec.thin(i);
    }
  }

  return StatusCode::SUCCESS;
}


} // namespace DMTest
