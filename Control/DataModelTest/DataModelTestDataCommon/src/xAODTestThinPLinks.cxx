/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestThinPLinks.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2024
 * @brief Thin PLinksContainer objects.
 */


#include "xAODTestThinPLinks.h"
#include "StoreGate/ThinningHandle.h"


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestThinPLinks::initialize()
{
  ATH_CHECK( m_plinksContainerKey.initialize (m_stream) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestThinPLinks::execute (const EventContext& ctx) const
{
  SG::ThinningHandle<DMTest::PLinksContainer> plinks(m_plinksContainerKey, ctx);

  unsigned int evt = ctx.evt();

  plinks.keepAll();
  for (size_t i = 0; i < plinks->size(); i++) {
    if (((evt ^ m_mask) & (1 << (i%4))) == 0) {
      plinks.thin(i);
    }
  }

  return StatusCode::SUCCESS;
}


} // namespace DMTest
